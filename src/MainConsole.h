#pragma once

#include "Console.h"
#include <string>

class ConsoleManager;

class MainConsole : public Console {
public:
    MainConsole(ConsoleManager& manager);
    void run() override;

private:
    void handleCommand(const std::string& input);

    ConsoleManager& consoleManager;
};