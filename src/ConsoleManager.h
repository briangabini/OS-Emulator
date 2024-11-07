#pragma once

#include "Config.h"
#include "Console.h"
#include "Process.h"
#include "Scheduler.h"
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>
#include <atomic>

class MainConsole;

class ConsoleManager {
public:
    ConsoleManager();
    ~ConsoleManager();

    void startCpuCycleCounter();
    void stopCpuCycleCounter();

    void start();
    void switchToMainConsole();
    void switchToScreen(Process* process);

    void createProcess(const std::string& name);
    Process* getProcess(const std::string& name);
    std::map<std::string, Process*>& getProcesses();

    Scheduler* getScheduler();

    void startScheduler();
    void stopScheduler();
    void pauseScheduler();
    void resumeScheduler();

    void startSchedulerTest();
    void stopSchedulerTest();
    void startScheduler10();

    // Console output management
    void safePrint(const std::string& message);
    void printPrompt();
    void setCurrentPrompt(const std::string& prompt);
    std::mutex& getIOMutex();

    bool isInitialized() const;
    bool initialize();

private:
    MainConsole* mainConsole;
    std::map<std::string, Process*> processes;
    std::mutex processMutex;

    // For CPU cycle functionality
    std::atomic<unsigned int> cpuCycles;
    std::thread cpuCycleThread;
    bool cpuCycleRunning;
    std::mutex cpuCycleMutex;
    std::condition_variable cpuCycleCV;
    void cpuCycleLoop();

    Scheduler* scheduler;

    // For scheduler test
    void schedulerTestLoop();
    std::thread testThread;
    bool testing;
    std::mutex testMutex;
    std::condition_variable testCV;

    // Console output management
    std::string currentPrompt;
    std::mutex ioMutex;

    bool initialized;
};
