#include "SchedulerRR.h"
#include "Command.h"
#include "Config.h"
#include <iostream>
#include <thread>
#include <algorithm>

SchedulerRR::SchedulerRR(int numCores, unsigned int quantum, ConsoleManager& manager)
    : numCores(numCores), quantum(quantum), running(false), paused(false), consoleManager(manager), cpuCycles(0) {

    for (int i = 0; i < numCores; ++i) {
        Worker* worker = new Worker();
        worker->coreId = i;
        workers.push_back(worker);
    }
}

SchedulerRR::~SchedulerRR() {
    stop();
    for (Worker* worker : workers) {
        delete worker;
    }
}

void SchedulerRR::addProcess(Process* process) {
    {
        std::lock_guard<std::mutex> lock(queuedProcessesMutex);
        if (queuedProcessesSet.find(process) == queuedProcessesSet.end()) {
            queuedProcessesSet.insert(process);
            processQueue.push(process);
        }
    }
    {
        std::lock_guard<std::mutex> lock(allProcessesMutex);
        if (std::find(allProcesses.begin(), allProcesses.end(), process) == allProcesses.end()) {
            allProcesses.push_back(process);
        }
    }
}

void SchedulerRR::start() {
    if (running.load()) return;
    running.store(true);
    paused.store(false);
    schedulerThread = std::thread(&SchedulerRR::schedulerLoop, this);
}

void SchedulerRR::stop() {
    if (!running.load()) return;
    running.store(false);
    paused.store(false);
    pauseCV.notify_all();
    processQueue.stop();
    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }

    for (Worker* worker : workers) {
        worker->cv.notify_all();
        if (worker->thread.joinable()) {
            worker->thread.join();
        }
    }
}

void SchedulerRR::pause() {
    if (!running.load() || paused.load()) return;
    paused.store(true);
}

void SchedulerRR::resume() {
    if (!running.load() || !paused.load()) return;
    paused.store(false);
    pauseCV.notify_all();
}

bool SchedulerRR::isRunning() const {
    return running.load();
}

bool SchedulerRR::isPaused() const {
    return paused.load();
}

void SchedulerRR::schedulerLoop() {
    for (Worker* worker : workers) {
        worker->thread = std::thread(&SchedulerRR::workerLoop, this, worker->coreId);
    }

    while (running.load()) {

        // Pause handling
        while (paused.load()) {
            if (!running.load()) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cpuCycles++;
            consoleManager.getMemoryManager().incrementIdleCpuTicks();
        }
        if (!running.load()) break;

        Process* process = nullptr;
        if (processQueue.wait_and_pop(process)) {
            if (!running.load()) break;

            // Remove from queuedProcessesSet
            {
                std::lock_guard<std::mutex> lock(queuedProcessesMutex);
                queuedProcessesSet.erase(process);
            }

            if (!process->isInMemory()) {
                // Process is not in memory, cannot schedule it
                // Try to allocate memory again
                if (!consoleManager.getMemoryManager().allocateMemory(process, process->getMemorySize())) {
                    // Requeue the process
                    addProcess(process);
                    continue;
                }
            }

            // Assign process to an idle worker
            bool assigned = false;
            while (!assigned && running.load()) {
                for (Worker* worker : workers) {
                    std::unique_lock<std::mutex> lock(worker->mtx);
                    if (!worker->busy.load()) {
                        worker->currentProcess = process;
                        worker->busy.store(true);
                        worker->remainingQuantum = quantum;
                        worker->cv.notify_one();
                        assigned = true;
                        break;
                    }
                }
                if (!assigned) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    cpuCycles++;
                    consoleManager.getMemoryManager().incrementIdleCpuTicks();
                }
            }
        }
    }
}

void SchedulerRR::workerLoop(int coreId) {
    Worker* worker = workers[coreId];
    Config& config = Config::getInstance();
    unsigned int delayPerExec = config.getDelaysPerExec();

    while (running.load()) {

        // Pause handling
        while (paused.load()) {
            if (!running.load()) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cpuCycles++;
            consoleManager.getMemoryManager().incrementIdleCpuTicks();
        }
        if (!running.load()) break;

        std::unique_lock<std::mutex> lock(worker->mtx);

        // Wait for a process to be assigned
        worker->cv.wait(lock, [worker, this]() {
            return worker->busy.load() || !running.load();
        });

        if (!running.load()) break;

        if (!worker->busy.load() || worker->currentProcess == nullptr) {
            // Spurious wakeup or process was set to nullptr
            continue;
        }

        Process* process = worker->currentProcess;
        unsigned int timeSlice = worker->remainingQuantum;

        lock.unlock();
        bool processCompleted = false;

        while (timeSlice > 0 && running.load()) {

            // Pause handling
            while (paused.load()) {
                if (!running.load()) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                cpuCycles++;
                consoleManager.getMemoryManager().incrementIdleCpuTicks();
            }
            if (!running.load()) break;

            Command* cmd = process->getNextCommand();
            if (cmd == nullptr) {
                process->setCompleted(true);
                process->log("Process finished execution.", coreId);

                // Deallocate memory
                consoleManager.getMemoryManager().deallocateMemory(process);

                processCompleted = true;
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            cpuCycles++;
            consoleManager.getMemoryManager().incrementActiveCpuTicks();

            // Execute instruction
            cmd->execute(process, coreId);
            delete cmd;

            process->incrementCurrentLine();

            // Simulate delay-per-exec
            for (unsigned int i = 0; i < delayPerExec; ++i) {
                cpuCycles++;
                consoleManager.getMemoryManager().incrementActiveCpuTicks();
            }

            // Decrement time slice
            timeSlice--;
            worker->remainingQuantum--;
        }

        if (!running.load()) break;

        lock.lock();
        worker->busy.store(false);
        worker->currentProcess = nullptr;
        worker->remainingQuantum = 0;
        lock.unlock();

        if (!processCompleted && !process->isCompleted()) {
            // Requeue the process
            addProcess(process);
        }
    }
}

int SchedulerRR::getTotalCores() const {
    return numCores;
}

int SchedulerRR::getBusyCores() const {
    int busyCores = 0;
    for (const Worker* worker : workers) {
        if (worker->busy.load()) {
            busyCores++;
        }
    }
    return busyCores;
}

std::map<Process*, int> SchedulerRR::getRunningProcesses() const {
    std::map<Process*, int> runningProcesses;
    for (const Worker* worker : workers) {
        if (worker->currentProcess != nullptr) {
            runningProcesses[worker->currentProcess] = worker->coreId;
        }
    }
    return runningProcesses;
}

std::vector<Process*> SchedulerRR::getQueuedProcesses() const {
    std::vector<Process*> queuedProcesses;
    {
        std::lock_guard<std::mutex> lock(queuedProcessesMutex);
        for (auto process : queuedProcessesSet) {
            queuedProcesses.push_back(process);
        }
    }
    return queuedProcesses;
}

std::vector<Process*> SchedulerRR::getFinishedProcesses() const {
    std::vector<Process*> finishedProcesses;
    std::lock_guard<std::mutex> lock(allProcessesMutex);
    auto runningProcesses = getRunningProcesses();
    for (Process* process : allProcesses) {
        if (process->isCompleted() && runningProcesses.find(process) == runningProcesses.end()) {
            finishedProcesses.push_back(process);
        }
    }
    return finishedProcesses;
}