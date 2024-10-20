#include "AScheduler.h"
#include <iostream>

AScheduler::AScheduler(SchedulingAlgorithm schedulingAlgo)
    : schedulingAlgo(schedulingAlgo)
{
    for (int i = 0; i < workersCount; i++)
    {
        // Initialize worker
        auto worker = std::make_shared<SchedulerWorker>(i, this);
        schedulerWorkers.push_back(worker);

        // Start the worker
        worker->start();
    }
}

void AScheduler::addProcess(std::shared_ptr<Process> process) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(process);
        std::cout << "Process #" << process->getPID() << " added. Queue size: " << readyQueue.size() << std::endl;
    }
    queueCV.notify_all(); // Notifies a waiting worker thread
}

void AScheduler::run()
{
    init();
    while (running)
    {
        execute();
    }
}

void AScheduler::stop()
{
    running = false;
}