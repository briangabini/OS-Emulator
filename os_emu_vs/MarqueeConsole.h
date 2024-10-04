#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <mutex>

class ConsoleManager;

class MarqueeConsole {
public:
    MarqueeConsole(ConsoleManager& manager);
    void run();

private:
    void marqueeLoop();
    void inputLoop();

    std::vector<std::string> messages;
    std::atomic<bool> running;
    ConsoleManager& consoleManager;
    std::mutex consoleMutex;
};