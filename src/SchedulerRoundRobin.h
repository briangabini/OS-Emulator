#pragma once

#include "Scheduler.h"
#include "Process.h"
#include "ConsoleManager.h"
#include <thread>
#include <vector>
#include <map>
#include <atomic>
#include <condition_variable>
#include <unordered_set>
#include "ThreadSafeQueue.h"

class SchedulerRoundRobin : public Scheduler {
public:
    SchedulerRoundRobin(int numCores, unsigned int quantum, ConsoleManager& manager);
    ~SchedulerRoundRobin();

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
    unsigned int quantum;

    std::vector<std::thread> workerThreads;
    std::thread schedulerThread;

    ThreadSafeQueue<Process*> processQueue;

    std::atomic<bool> running;
    std::atomic<bool> paused;
    std::mutex pauseMutex;
    std::condition_variable pauseCV;

    struct Worker {
        int coreId = 0;
        std::atomic<bool> busy{ false };
        Process* currentProcess = nullptr;
        unsigned int remainingQuantum = 0;
        std::thread thread;
        std::mutex mtx;
        std::condition_variable cv;
    };

    std::vector<Worker*> workers;

    ConsoleManager& consoleManager;

    std::vector<Process*> allProcesses;
    mutable std::mutex allProcessesMutex;

    std::atomic<unsigned int> cpuCycles;

    std::unordered_set<Process*> queuedProcessesSet;
    mutable std::mutex queuedProcessesMutex;
};