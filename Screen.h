#pragma once

#include "Console.h"
#include "Process.h"
#include <string>

class ConsoleManager;

class Screen : public Console {
public:
    Screen(ConsoleManager& manager, Process* process);
    void run() override;

private:
    void displayProcessScreen();

    ConsoleManager& consoleManager;
    Process* process;
};