#pragma once

#include "Console.h"
#include <string>
#include <vector>
#include <atomic>

class ConsoleManager;

class MarqueeNT : public Console {
public:
    MarqueeNT(ConsoleManager& manager);
    ~MarqueeNT();
    void run() override;

private:
    void clearMarqueeLines();

    std::vector<std::string> messages;
    std::atomic<bool> running;

    ConsoleManager& consoleManager;
};