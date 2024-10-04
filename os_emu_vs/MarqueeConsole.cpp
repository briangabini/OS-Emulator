#include "MarqueeConsole.h"
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <chrono>
#include <thread>
#include <mutex>

MarqueeConsole::MarqueeConsole(ConsoleManager& manager) : consoleManager(manager), running(true) {
    messages = {
        R"(  _   _      _ _         __        __         _     _ )",
        R"( | | | | ___| | | ___    \ \      / /__  _ __| | __| |)",
        R"( | |_| |/ _ \ | |/ _ \    \ \ /\ / / _ \| '__| |/ _` |)",
        R"( |  _  |  __/ | | (_) |    \ V  V / (_) | |  | | (_| |)",
        R"( |_| |_|\___|_|_|\___/      \_/\_/ \___/|_|  |_|\__,_|)"
    };
}

void MarqueeConsole::run() {
    system("cls");
    std::thread marqueeThread(&MarqueeConsole::marqueeLoop, this);
    inputLoop();
    running = false;
    marqueeThread.join();
}

void MarqueeConsole::marqueeLoop() {
    const int MARQUEE_WIDTH = 60;
    std::vector<size_t> start_positions(messages.size(), 0);

    while (running) {
        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(hConsole, &csbi);
            COORD cursorPosition = csbi.dwCursorPosition;

            for (size_t i = 0; i < messages.size(); ++i) {
                SetConsoleCursorPosition(hConsole, { 0, static_cast<SHORT>(i) });
                std::string to_display = messages[i].substr(start_positions[i], MARQUEE_WIDTH);
                if (to_display.length() < MARQUEE_WIDTH) {
                    to_display += messages[i].substr(0, MARQUEE_WIDTH - to_display.length());
                }
                std::cout << to_display;
            }

            SetConsoleCursorPosition(hConsole, cursorPosition);
        }

        for (size_t i = 0; i < messages.size(); ++i) {
            start_positions[i] = (start_positions[i] + 1) % messages[i].length();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void MarqueeConsole::inputLoop() {
    std::string user_input;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hConsole, { 0, static_cast<SHORT>(messages.size() + 1) });
    std::cout << "\nMarquee> ";
    std::cout.flush();

    while (running) {
        if (_kbhit()) {
            char ch = _getch();

            if (ch == '\r') { // Enter key
                {
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    SetConsoleCursorPosition(hConsole, { 0, static_cast<SHORT>(messages.size() + 2) });
                    std::cout << "You entered: " << user_input << std::string(60 - user_input.length(), ' ') << "\n";
                }

                if (user_input == "back" || user_input == "exit") {
                    running = false;
                    break;
                }

                user_input.clear();
                SetConsoleCursorPosition(hConsole, { 0, static_cast<SHORT>(messages.size() + 3) });
                std::cout << "Marquee> " << std::string(60, ' ');
                SetConsoleCursorPosition(hConsole, { 8, static_cast<SHORT>(messages.size() + 3) });
            }
            else if (ch == '\b') { // Backspace key
                if (!user_input.empty()) {
                    user_input.pop_back();
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    std::cout << "\b \b";
                }
            }
            else {
                user_input += ch;
                std::lock_guard<std::mutex> lock(consoleMutex);
                std::cout << ch;
            }
            std::cout.flush();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "\nReturning to main console...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
}