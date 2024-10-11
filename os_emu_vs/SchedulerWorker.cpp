// SchedulerWorker.cpp
#include "SchedulerWorker.h"
#include <iostream>

void SchedulerWorker::update(bool isRunning) {
    this->running = isRunning;
}

void SchedulerWorker::assignProcess(std::shared_ptr<Process> process) {
    {
        std::lock_guard<std::mutex> lock(processMutex);
        this->currentProcess = process;
        this->update(true);
    }
    processCV.notify_one();
}

void SchedulerWorker::run() {
    while (true) {
        std::unique_lock<std::mutex> lock(processMutex);
        processCV.wait(lock, [this] { return currentProcess != nullptr || !running; });

        if (currentProcess) {
            currentProcess->setState(currentProcess->RUNNING);
            currentProcess->setCpuCoreId(cpuCoreId);


            while (!currentProcess->isFinished() && running) {
                currentProcess->executeCurrentCommand();
                currentProcess->moveToNextLine();
                IETThread::sleep(50);                   // this is here because printing is trivial (constant time)
            }
            currentProcess->setState(currentProcess->FINISHED);
            currentProcess = nullptr;
			this->update(false);
        }
    }
}

bool SchedulerWorker::isRunning() {
    return running;
}
