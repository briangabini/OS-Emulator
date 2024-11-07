#include "SchedulerFCFS.h"
#include "Command.h"
#include <iostream>

SchedulerFCFS::SchedulerFCFS(int numCores, ConsoleManager& manager)
    : numCores(numCores), running(false), paused(false), consoleManager(manager) {
    
    for (int i = 0; i < numCores; ++i) {
        Worker* worker = new Worker();
        worker->coreId = i;
        workers.push_back(worker);
    }
}

SchedulerFCFS::~SchedulerFCFS() {
    stop();
    for (Worker* worker : workers) {
        delete worker;
    }
}

void SchedulerFCFS::addProcess(Process* process) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        processQueue.push(process);
        queueCV.notify_one();
    }
    {
        std::lock_guard<std::mutex> lock(allProcessesMutex);
        allProcesses.push_back(process);
    }
}

void SchedulerFCFS::start() {
    if (running) return;
    running = true;
    paused = false;
    schedulerThread = std::thread(&SchedulerFCFS::schedulerLoop, this);
}

void SchedulerFCFS::stop() {
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

void SchedulerFCFS::pause() {
    std::unique_lock<std::mutex> lock(pauseMutex);
    if (!running || paused) return;
    paused = true;
}

void SchedulerFCFS::resume() {
    std::unique_lock<std::mutex> lock(pauseMutex);
    if (!running || !paused) return;
    paused = false;
    pauseCV.notify_all();
}

bool SchedulerFCFS::isRunning() const {
    return running;
}

bool SchedulerFCFS::isPaused() const {
    return paused;
}

void SchedulerFCFS::schedulerLoop() {
    for (Worker* worker : workers) {
        worker->thread = std::thread(&SchedulerFCFS::workerLoop, this, worker->coreId);
    }

    while (running) {

        // For pausing
        {
            std::unique_lock<std::mutex> lock(pauseMutex);
            pauseCV.wait(lock, [this]() { return !paused || !running; });
            if (!running) break;
        }

        std::unique_lock<std::mutex> lock(queueMutex);
        queueCV.wait(lock, [this]() { return !processQueue.empty() || !running; });
        if (!running) break;

        // Assign processes to idle workers
        for (Worker* worker : workers) {
            if (!worker->busy && !processQueue.empty()) {
                Process* process = processQueue.front();
                processQueue.pop();

                {
                    std::lock_guard<std::mutex> workerLock(worker->mtx);
                    worker->currentProcess = process;
                    worker->busy = true;
                }

                worker->cv.notify_one();
            }
        }
    }
}

void SchedulerFCFS::workerLoop(int coreId) {
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

        // Execute the process's commands
        Command* cmd = nullptr;
        while ((cmd = process->getNextCommand()) != nullptr) {

            // For pausing
            {
                std::unique_lock<std::mutex> pauseLock(pauseMutex);
                pauseCV.wait(pauseLock, [this]() { return !paused || !running; });
                if (!running) break;
            }

            process->incrementCurrentLine();

            cmd->execute(process, coreId);
            delete cmd;

            // Delay before executing the next instruction
            unsigned int delay = Config::getInstance().getDelaysPerExec();
            if (delay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }
        }

        // Process finished
        process->setCompleted(true);
        process->log("Process finished execution.", coreId);

        // Indicate worker is idle
        lock.lock();
        worker->busy = false;
        worker->currentProcess = nullptr;
    }
}

int SchedulerFCFS::getTotalCores() const {
    return numCores;
}

int SchedulerFCFS::getBusyCores() const {
    int busyCores = 0;
    for (const Worker* worker : workers) {
        std::lock_guard<std::mutex> lock(worker->mtx);
        if (worker->busy) {
            busyCores++;
        }
    }
    return busyCores;
}

std::map<Process*, int> SchedulerFCFS::getRunningProcesses() const {
    std::map<Process*, int> runningProcesses;
    for (const Worker* worker : workers) {
        std::lock_guard<std::mutex> lock(worker->mtx);
        if (worker->currentProcess != nullptr) {
            runningProcesses[worker->currentProcess] = worker->coreId;
        }
    }
    return runningProcesses;
}

std::vector<Process*> SchedulerFCFS::getQueuedProcesses() const {
    std::vector<Process*> queuedProcesses;
    std::unique_lock<std::mutex> lock(queueMutex);
    std::queue<Process*> tempQueue = processQueue;
    while (!tempQueue.empty()) {
        queuedProcesses.push_back(tempQueue.front());
        tempQueue.pop();
    }
    return queuedProcesses;
}

std::vector<Process*> SchedulerFCFS::getFinishedProcesses() const {
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
