#pragma once

#include "Config.h"
#include "Console.h"
#include "Process.h"
#include "Scheduler.h"
#include "MemoryManager.h"
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <sstream>

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

    bool createProcess(const std::string& name);
    Process* getProcess(const std::string& name);
    std::map<std::string, Process*>& getProcesses();

    MemoryManager& getMemoryManager();
    Scheduler* getScheduler();

    void startScheduler();
    void stopScheduler();
    void pauseScheduler();
    void resumeScheduler();

    // Scheduler test methods
    void startSchedulerTest();
    void startSchedulerTestWithProcesses(int numProcesses);
    void startSchedulerTestWithDuration(int seconds);
    void stopSchedulerTest();

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

    MemoryManager memoryManager;
    Scheduler* scheduler;

    // For scheduler test
    void schedulerTestLoop();
    void generateTestProcess(const std::string& baseName, std::stringstream* outputStream = nullptr);
    std::thread testThread;
    bool testing;
    std::mutex testMutex;
    std::condition_variable testCV;
    std::atomic<int> processCounter{ 1 };

    // Console output management
    std::string currentPrompt;
    std::mutex ioMutex;

    bool initialized;
};