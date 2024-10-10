#pragma once

#include "Console.h"

class ConsoleManager;
class Process;

class Screen : public Console {
public:
    Screen(ConsoleManager& manager, Process* process);
    void run() override;

private:
    void displayProcessScreen();

    ConsoleManager& consoleManager;
    Process* process;
};
