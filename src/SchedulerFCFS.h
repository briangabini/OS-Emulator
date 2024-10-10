#pragma once

#include "Scheduler.h"
#include "Process.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

class SchedulerFCFS : public Scheduler {
public:
    SchedulerFCFS(int numCores);
    ~SchedulerFCFS();

    void addProcess(Process* process) override;
    void start() override;
    void stop() override;

private:
    void schedulerLoop();
    void workerLoop(int coreId);

    int numCores;
    std::vector<std::thread> workerThreads;
    std::thread schedulerThread;

    std::queue<Process*> processQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;

    bool running;

    struct Worker {
        int coreId = 0;
        bool busy = false;
        Process* currentProcess = nullptr;
        std::thread thread;
        std::mutex mtx;
        std::condition_variable cv;
    };

    std::vector<Worker*> workers;
};
