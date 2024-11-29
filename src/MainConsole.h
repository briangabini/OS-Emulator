#pragma once

#include "Console.h"
#include <string>
#include <vector>
#include <map>

class ConsoleManager;
class Process;

class MainConsole : public Console {
public:
    MainConsole(ConsoleManager& manager);
    void run() override;

private:
    void handleCommand(const std::string& input);

    void displayProcessSmi();
    void displayVmStat();
    void displayRunningProcesses(const std::vector<Process*>& runningProcesses, const std::map<Process*, int>& runningProcessesMap);
    void displayFinishedProcesses(const std::vector<Process*>& finishedProcesses);
    void displayQueuedProcesses(const std::vector<Process*>& queuedProcesses);

    void reportUtil();

    ConsoleManager& consoleManager;
};
