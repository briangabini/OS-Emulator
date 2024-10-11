#include "AScheduler.h"
#include <iostream>

AScheduler::AScheduler(SchedulingAlgorithm schedulingAlgo)
    : schedulingAlgo(schedulingAlgo)
{
    for (int i = 0; i < workersCount; i++)
    {
        // Initialize worker
        auto worker = std::make_shared<SchedulerWorker>(i);
        schedulerWorkers.push_back(worker);

        // Start the worker
        worker->start();
    }
}

void AScheduler::addProcess(std::shared_ptr<Process> process)
{
    readyQueue.push(process);
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