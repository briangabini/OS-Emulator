// FCFSScheduler.cpp
#include "FCFSScheduler.h"
#include "SchedulerWorker.h"
#include <iostream>

FCFSScheduler::FCFSScheduler()
    : AScheduler(SchedulingAlgorithm::FCFS) {
}

void FCFSScheduler::init() {
    // Additional setup if needed
}

std::shared_ptr<SchedulerWorker> FCFSScheduler::findAvailableWorker() {
    for (auto& worker : schedulerWorkers) {
        if (!worker->isRunning()) {
            return worker;
        }
    }
    return nullptr;
}

void FCFSScheduler::execute() {
    while (running) {
        std::shared_ptr<Process> currentProcess = nullptr;

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!readyQueue.empty()) {
                currentProcess = readyQueue.front();
                readyQueue.pop();
            }
        }

        if (currentProcess) {
            std::shared_ptr<SchedulerWorker> worker = nullptr;

            while (!(worker = findAvailableWorker())) {
                IETThread::sleep(200);
            }

            worker->assignProcess(currentProcess);
        }
    }
}
