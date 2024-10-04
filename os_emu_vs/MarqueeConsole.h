#pragma once

#include "BaseScreen.h"
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>

class MarqueeConsole : public BaseScreen {
public:
    MarqueeConsole(const std::shared_ptr<Process>& process, const std::string& name);
    ~MarqueeConsole();

    void setMarqueeText(const std::string& newText);
    void start(bool threaded = true);
    void stop();
    void setRefreshRate(int rate);
    void processInput(const std::string& input);
    void run(); 

    bool isRunning() const { return running; }

private:
    void marqueeWorker();
    void updateMarquee();
    void clearScreen();
    int getConsoleWidth() const;

    std::atomic<bool> running;
    std::string text;
    size_t position;
    std::thread workerThread;
    int refreshRate;
    bool isThreaded;
    std::function<bool()> exitCondition;
};