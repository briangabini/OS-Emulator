#pragma once

#include "Console.h"
#include "Process.h"
#include "Scheduler.h"
#include <map>
#include <mutex>
#include <string>

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

    std::mutex& getIOMutex();

private:
    MainConsole* mainConsole;
    std::map<std::string, Process*> processes;
    std::mutex processMutex;
    std::mutex ioMutex;

    Scheduler* scheduler;
};
