#include "Console.h"
#include <string>
#include <vector>
#include <thread>
#include <atomic>

class ConsoleManager;

class Marquee : public Console {
public:
    Marquee(ConsoleManager& manager);
    ~Marquee();
    void run() override;

private:
    void marqueeLoop();
    void inputLoop();
    void clearMarqueeLines();

    std::vector<std::string> messages;
    std::atomic<bool> running;
    std::thread marqueeThread;
    std::thread inputThread;

    ConsoleManager& consoleManager;
};