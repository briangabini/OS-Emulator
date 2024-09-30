#pragma once
#include "AConsole.h"
#include <string>
#include <chrono>
#include <thread>
#include <atomic>

class MarqueeConsole : public AConsole {
public:
    MarqueeConsole();
    ~MarqueeConsole() override;
    void onEnabled() override;
    void process() override;
    void display() override;

private:
    std::string marqueeText;
    int position;
    int screenWidth;
    std::atomic<bool> running;
    std::thread inputThread;
    std::string userInput;

    void updateMarquee();
    void handleInput();
    void clearScreen();
};
