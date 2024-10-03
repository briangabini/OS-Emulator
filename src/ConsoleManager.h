#pragma once

#include "Console.h"
#include "MainConsole.h"
#include "Process.h"
#include "Screen.h"
#include <string>
#include <map>
#include <mutex>

class MainConsole;
class Process;
class Screen;

class ConsoleManager {
public:
    ConsoleManager();
    ~ConsoleManager();

    void start();
    void switchToMainConsole();
    void switchToScreen(Process* process);
    void switchToMarquee();
    std::mutex& getIOMutex();

    void createProcess(const std::string& name);
    Process* getProcess(const std::string& name);

private:
    MainConsole* mainConsole;
    std::map<std::string, Process*> processes;
    std::mutex processMutex;
    std::mutex ioMutex;
};