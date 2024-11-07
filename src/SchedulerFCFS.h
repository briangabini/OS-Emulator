#pragma once

#include "Scheduler.h"
#include "Process.h"
#include "ConsoleManager.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <map>

class SchedulerFCFS : public Scheduler {
public:
    SchedulerFCFS(int numCores, ConsoleManager& manager);
    ~SchedulerFCFS();

    void addProcess(Process* process) override;
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    bool isRunning() const override;
    bool isPaused() const override;

    int getTotalCores() const override;
    int getBusyCores() const override;

    std::map<Process*, int> getRunningProcesses() const override;
    std::vector<Process*> getQueuedProcesses() const override;
    std::vector<Process*> getFinishedProcesses() const override;

private:
    void schedulerLoop();
    void workerLoop(int coreId);

    int numCores;
    std::vector<std::thread> workerThreads;
    std::thread schedulerThread;

    mutable std::mutex queueMutex;
    std::condition_variable queueCV;
    std::queue<Process*> processQueue;

    bool running;
    bool paused;
    mutable std::mutex pauseMutex;
    std::condition_variable pauseCV;

    struct Worker {
        int coreId = 0;
        bool busy = false;
        Process* currentProcess = nullptr;
        std::thread thread;
        mutable std::mutex mtx;
        std::condition_variable cv;
    };

    std::vector<Worker*> workers;

    ConsoleManager& consoleManager;

    std::vector<Process*> allProcesses;
    mutable std::mutex allProcessesMutex;
};
