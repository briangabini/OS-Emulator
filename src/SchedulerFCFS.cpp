#include "SchedulerFCFS.h"
#include "Command.h"
#include "Process.h"
#include <iostream>

SchedulerFCFS::SchedulerFCFS(int numCores)
    : numCores(numCores), running(false) {
    // Initialize workers
    for (int i = 0; i < numCores; ++i) {
        Worker* worker = new Worker();
        worker->coreId = i;
        worker->busy = false;
        worker->currentProcess = nullptr;
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
    std::unique_lock<std::mutex> lock(queueMutex);
    processQueue.push(process);
    queueCV.notify_one();
}

void SchedulerFCFS::start() {
    running = true;
    schedulerThread = std::thread(&SchedulerFCFS::schedulerLoop, this);
}

void SchedulerFCFS::stop() {
    running = false;
    queueCV.notify_all();
    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }
    // Notify worker threads to stop
    for (Worker* worker : workers) {
        worker->cv.notify_all();
        if (worker->thread.joinable()) {
            worker->thread.join();
        }
    }
}

void SchedulerFCFS::schedulerLoop() {
    // Start worker threads
    for (Worker* worker : workers) {
        worker->thread = std::thread(&SchedulerFCFS::workerLoop, this, worker->coreId);
    }

    while (running) {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCV.wait(lock, [this]() { return !processQueue.empty() || !running; });
        if (!running) break;

        // Assign processes to idle workers
        for (Worker* worker : workers) {
            if (!worker->busy) {
                if (!processQueue.empty()) {
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
}

void SchedulerFCFS::workerLoop(int coreId) {
    Worker* worker = workers[coreId];
    while (running) {
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
            {
                // Update current line before executing the command
                process->incrementCurrentLine();
            }

            cmd->execute(process, coreId);
            delete cmd;
        }

        // Process finished
        {
            process->setCompleted(true);
        }

        process->log("Process finished execution.", coreId);

        // Indicate worker is idle
        lock.lock();
        worker->busy = false;
        worker->currentProcess = nullptr;
    }
}
