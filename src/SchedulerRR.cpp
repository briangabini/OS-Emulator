#include "SchedulerRR.h"
#include "Command.h"
#include "Config.h"
#include <iostream>

SchedulerRR::SchedulerRR(int numCores, unsigned int quantum, ConsoleManager& manager)
    : numCores(numCores), quantum(quantum), running(false), paused(false), consoleManager(manager) {
    
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
    std::unique_lock<std::mutex> lock(queueMutex);
    processQueue.push(process);
    queueCV.notify_one();
}

void SchedulerRR::start() {
    if (running) return;
    running = true;
    paused = false;
    schedulerThread = std::thread(&SchedulerRR::schedulerLoop, this);
}

void SchedulerRR::stop() {
    if (!running) return;
    running = false;
    paused = false;
    queueCV.notify_all();
    pauseCV.notify_all();
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
    std::unique_lock<std::mutex> lock(pauseMutex);
    if (!running || paused) return;
    paused = true;
}

void SchedulerRR::resume() {
    std::unique_lock<std::mutex> lock(pauseMutex);
    if (!running || !paused) return;
    paused = false;
    pauseCV.notify_all();
}

bool SchedulerRR::isRunning() const {
    return running;
}

bool SchedulerRR::isPaused() const {
    return paused;
}

void SchedulerRR::schedulerLoop() {
    for (Worker* worker : workers) {
        worker->thread = std::thread(&SchedulerRR::workerLoop, this, worker->coreId);
    }

    while (running) {

        // For pausing
        {
            std::unique_lock<std::mutex> lock(pauseMutex);
            pauseCV.wait(lock, [this]() { return !paused || !running; });
            if (!running) break;
        }

        // Check for idle workers and assign processes
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            for (Worker* worker : workers) {
                std::unique_lock<std::mutex> wlock(worker->mtx);
                if (!worker->busy && !processQueue.empty()) {
                    Process* process = processQueue.front();
                    processQueue.pop();

                    worker->currentProcess = process;
                    worker->busy = true;
                    worker->remainingQuantum = quantum;

                    worker->cv.notify_one();
                }
            }
        }

        // Prevent busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void SchedulerRR::workerLoop(int coreId) {
    Worker* worker = workers[coreId];
    while (running) {

        // For pausing
        {
            std::unique_lock<std::mutex> lock(pauseMutex);
            pauseCV.wait(lock, [this]() { return !paused || !running; });
            if (!running) break;
        }

        std::unique_lock<std::mutex> lock(worker->mtx);
        if (!worker->busy) {
            worker->cv.wait(lock, [worker, this]() { return worker->busy || !running; });
        }
        if (!running) break;
        Process* process = worker->currentProcess;
        lock.unlock();

        if (process == nullptr) {
            continue;
        }

        // Execute commands up to the quantum limit
        unsigned int timeSlice = worker->remainingQuantum;
        bool processCompleted = false;

        while (timeSlice > 0) {
            Command* cmd = process->getNextCommand();
            if (cmd == nullptr) {
                process->setCompleted(true);
                process->log("Process finished execution.", coreId);
                processCompleted = true;
                break;
            }

            // For pausing
            {
                std::unique_lock<std::mutex> pauseLock(pauseMutex);
                pauseCV.wait(pauseLock, [this]() { return !paused || !running; });
                if (!running) {
                    delete cmd;
                    break;
                }
            }

            process->incrementCurrentLine();

            cmd->execute(process, coreId);
            delete cmd;

            // Delay before executing the next instruction
            unsigned int delay = Config::getInstance().getDelaysPerExec();
            if (delay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }

            timeSlice--;
            worker->remainingQuantum--;
        }

        if (!running) break;

        lock.lock();
        worker->busy = false;
        worker->currentProcess = nullptr;
        worker->remainingQuantum = 0;
        lock.unlock();

        if (!processCompleted && !process->isCompleted()) {
            // Requeue the process
            std::unique_lock<std::mutex> qlock(queueMutex);
            processQueue.push(process);
        }
    }
}

int SchedulerRR::getTotalCores() const {
    return numCores;
}

int SchedulerRR::getBusyCores() const {
    int busyCores = 0;
    for (const Worker* worker : workers) {
        std::lock_guard<std::mutex> lock(worker->mtx);
        if (worker->busy) {
            busyCores++;
        }
    }
    return busyCores;
}

std::map<Process*, int> SchedulerRR::getRunningProcesses() const {
    std::map<Process*, int> runningProcesses;
    for (const Worker* worker : workers) {
        std::lock_guard<std::mutex> lock(worker->mtx);
        if (worker->currentProcess != nullptr) {
            runningProcesses[worker->currentProcess] = worker->coreId;
        }
    }
    return runningProcesses;
}
