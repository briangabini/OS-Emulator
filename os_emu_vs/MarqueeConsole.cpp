#include "MarqueeConsole.h"
#include <iostream>
#include <windows.h>
#include <conio.h>

MarqueeConsole::MarqueeConsole(const std::shared_ptr<Process>& process, const std::string& name)
    : BaseScreen(process, name), running(false), text(""), position(0), refreshRate(30), isThreaded(true) {}

MarqueeConsole::~MarqueeConsole() {
    stop();
}

void MarqueeConsole::setMarqueeText(const std::string& newText) {
    text = newText;
    position = 0;
}

void MarqueeConsole::start(bool threaded) {
    running = true;
    isThreaded = threaded;
    exitCondition = []() {
        return _kbhit() && _getch() == 27; // Check for ESC key
        };

    if (isThreaded) {
        workerThread = std::thread(&MarqueeConsole::marqueeWorker, this);
    }
    else {
        run();
    }
}

void MarqueeConsole::stop() {
    running = false;
    if (isThreaded && workerThread.joinable()) {
        workerThread.join();
    }
}

void MarqueeConsole::setRefreshRate(int rate) {
    refreshRate = rate;
}

void MarqueeConsole::marqueeWorker() {
    while (running && !exitCondition()) {
        updateMarquee();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / refreshRate));
    }
    running = false;
}

void MarqueeConsole::run() {
    while (running && !exitCondition()) {
        updateMarquee();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / refreshRate));
    }
    running = false;
}

void MarqueeConsole::updateMarquee() {
    clearScreen();

    // Display header
    std::cout << "**********************************************\n";
    std::cout << "* Displaying a marquee console! *\n";
    std::cout << "**********************************************\n\n";

    int consoleWidth = getConsoleWidth();
    std::string visibleText = text + " " + text;
    visibleText = visibleText.substr(position, consoleWidth);

    // Print the visible portion of the text
    std::cout << visibleText << "\n\n";

    // Display footer
    std::cout << "Enter a command for MARQUEE_CONSOLE: Notice the crude refresh. This is very dependent on your monitor\n";
    std::cout << "Command processed in MARQUEE_CONSOLE: This is a sample barebones immediate mode UI drawing.\n";
    std::cout << "Press ESC to exit the marquee.\n";

    // Move the position for the next update
    position = (position + 1) % text.length();
}

void MarqueeConsole::processInput(const std::string& input) {
    if (input == "exit") {
        running = false;
    }
    else {
        std::cout << "\nProcessed command: " << input << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void MarqueeConsole::clearScreen() {
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(console, topLeft);
}

int MarqueeConsole::getConsoleWidth() const {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}