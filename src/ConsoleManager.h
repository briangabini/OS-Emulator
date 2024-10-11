#pragma once

#include "Console.h"
#include "Process.h"
#include "Scheduler.h"
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>

class MainConsole;

class ConsoleManager {
public:
    ConsoleManager();
    ~ConsoleManager();

    void start();
    void switchToMainConsole();
    void switchToScreen(Process* process);
    void switchToMarquee();
    void switchToMarqueeNT();

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

private:
    MainConsole* mainConsole;
    std::map<std::string, Process*> processes;
    std::mutex processMutex;

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
};
