#pragma once
#include "IETThread.h"
#include "Process.h"
#include "AScheduler.h"
#include <memory>
#include <mutex>
#include <condition_variable>

class AScheduler;

class SchedulerWorker : public IETThread {
public:
    SchedulerWorker(int cpuCoreId, AScheduler* scheduler)
        : cpuCoreId(cpuCoreId), scheduler(scheduler) {}
    ~SchedulerWorker() = default;

    void update(bool running);
    void run() override;
    bool isRunning();

private:
    int cpuCoreId;
    AScheduler* scheduler; // Pointer to the scheduler for direct access to readyQueue and queueMutex
    std::shared_ptr<Process> currentProcess = nullptr;
    bool running = true;
    std::mutex processMutex;
    std::condition_variable processCV;
};

