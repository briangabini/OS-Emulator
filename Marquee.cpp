#include "Marquee.h"
#include "ConsoleManager.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

Marquee::Marquee(ConsoleManager& manager)
    : running(true), consoleManager(manager) {
    /*
    messages = {
        "Welcome to the OS Emulator! Welcome to the OS Emulator! Welcome to the OS Emulator!",
        "Enjoy the scrolling marquee. Enjoy the scrolling marquee. Enjoy the scrolling marquee.",
        "This is line three of marquee. This is line three of marquee. This is line three of marquee.",
        "Another scrolling line here. Another scrolling line here. Another scrolling line here.",
        "Final marquee line scrolling. Final marquee line scrolling. Final marquee line scrolling."
    };
    */

    messages = {
        R"(  _____   _____    _____   _____    _______   _____   __    __          _____   _____    _____   _____    _______   _____   __    __)",
        R"( / ____| / ____|  /  __ \  |  __ \  |  ____| / ____|  \ \  / /         / ____| / ____|  /  __ \  |  __ \  |  ____| / ____|  \ \  / /)",
        R"(| |      | (___   | |  | | | |__) | | |__    | (___    \ \/ /         | |      | (___   | |  | | | |__) | | |__    | (___    \ \/ / )",
        R"(| |       \___ \  | |  | | |  ___/  |  __|    \___ \    |  |          | |       \___ \  | |  | | |  ___/  |  __|    \___ \    |  |  )",
        R"(| |____   ____) | | |__| | | |      | |____   ____) |   |  |          | |____   ____) | | |__| | | |      | |____   ____) |   |  |  )",
        R"( \_____| |_____/  \_____/  |_|      |______| |_____/    |__|           \_____| |_____/  \_____/  |_|      |______| |_____/    |__|  )"
    };
}

Marquee::~Marquee() {
    running = false;
    if (marqueeThread.joinable()) {
        marqueeThread.join();
    }
    if (inputThread.joinable()) {
        inputThread.join();
    }
}

void Marquee::run() {
    marqueeThread = std::thread(&Marquee::marqueeLoop, this);
    inputThread = std::thread(&Marquee::inputLoop, this);

    marqueeThread.join();
    inputThread.join();
}

void Marquee::marqueeLoop() {
    const int MARQUEE_WIDTH = 100;
    const int NUM_LINES = 6;
    std::vector<std::string> displays(NUM_LINES);
    std::vector<size_t> start_positions(NUM_LINES, 0);
    std::vector<size_t> msgLengths(NUM_LINES);

    for (int i = 0; i < NUM_LINES; ++i) {
        messages[i] += "        "; // Spaces in between messages
        msgLengths[i] = messages[i].length();
    }

    while (running) {
        {
            std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());

            // Save cursor position
#ifdef _WIN32
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleScreenBufferInfo(hConsole, &csbi);
            COORD originalCursorPosition = csbi.dwCursorPosition;
#else
            std::cout << "\033[s"; // Save cursor position
#endif

            // Update each marquee line
            for (int line = 0; line < NUM_LINES; ++line) {
                // Move cursor to the correct line
#ifdef _WIN32
                COORD marqueePosition = { 0, (SHORT)line };
                SetConsoleCursorPosition(hConsole, marqueePosition);
#else
                std::cout << "\033[" << (line + 1) << ";1H"; // Move cursor to line (line + 1), column 1
#endif

                // Clear the marquee line
                clearMarqueeLines();

                // Get the substring to display
                std::string to_display = messages[line].substr(start_positions[line], MARQUEE_WIDTH);
                if (to_display.length() < MARQUEE_WIDTH) {
                    to_display += messages[line].substr(0, MARQUEE_WIDTH - to_display.length());
                }

                std::cout << to_display;
            }

            std::cout.flush();

            // Restore cursor position
#ifdef _WIN32
            SetConsoleCursorPosition(hConsole, originalCursorPosition);
#else
            std::cout << "\033[u";
#endif
        }

        // Update start positions for scrolling
        for (int i = 0; i < NUM_LINES; ++i) {
            start_positions[i] = (start_positions[i] + 1) % msgLengths[i];
        }

        std::this_thread::sleep_for(std::chrono::microseconds(8333)); // Optimized for 120Hz display
    }
}

void Marquee::inputLoop() {
    std::string user_input;

    // Move cursor below the marquee lines and display the prompt
    {
        std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        COORD inputPosition = { 0, (SHORT)7 }; // Below the 5 marquee lines with a space
        SetConsoleCursorPosition(hConsole, inputPosition);
#else
        std::cout << "\033[" << (5 + 1) << ";1H"; // Move cursor to line 6, column 1
#endif
        std::cout << "Marquee> ";
        std::cout.flush();
    }

#ifdef _WIN32
    // Windows implementation using _kbhit() and _getch()
    while (running) {
        if (_kbhit()) {
            char ch = _getch();

            if (ch == '\r') { // Enter key
                {
                    std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                    std::cout << "\nMarquee> You entered: " << user_input << "\n";
                    std::cout << "Marquee> ";
                    std::cout.flush();
                }

                if (user_input == "back") {
                    running = false;
                    break;
                }
                else if (user_input == "exit") {
                    running = false;
                    break;
                }
                else {
                    // Handle other commands if needed
                }

                user_input.clear();
            }
            else if (ch == '\b' || ch == 127) { // Backspace key
                if (!user_input.empty()) {
                    user_input.pop_back();
                    {
                        std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                        std::cout << "\b \b";
                        std::cout.flush();
                    }
                }
            }
            else {
                user_input += ch;
                {
                    std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                    std::cout << ch;
                    std::cout.flush();
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(8333)); // Optimized for 120Hz display
    }
    std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
    std::cout << "Exiting the Marquee Console..." << std::endl;
    std::cout.flush();
#else
    // POSIX implementation using termios
    // Configure terminal for non-blocking input
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // Save old settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Set non-blocking read
    int oldf = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    while (running) {
        char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n > 0) {
            if (ch == '\n') { // Enter key
                {
                    std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                    std::cout << "\nYou entered: " << user_input << "\n";
                    std::cout << "Marquee> ";
                    std::cout.flush();
                }

                if (user_input == "back") {
                    running = false;
                    break;
                }
                else if (user_input == "exit") {
                    running = false;
                    break;
                }
                else {
                    // Handle other commands if needed
                }

                user_input.clear();
            }
            else if (ch == 127 || ch == '\b') { // Backspace key
                if (!user_input.empty()) {
                    user_input.pop_back();
                    {
                        std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                        std::cout << "\b \b";
                        std::cout.flush();
                    }
                }
            }
            else {
                user_input += ch;
                {
                    std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                    std::cout << ch;
                    std::cout.flush();
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
    std::cout << "Exiting the Marquee Console..." << std::endl;
    std::cout.flush();

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
#endif
}

void Marquee::clearMarqueeLines() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    COORD lineStart = csbi.dwCursorPosition;
    FillConsoleOutputCharacter(hConsole, ' ', csbi.dwSize.X, lineStart, &dwWritten);
#else
    std::cout << "\033[2K"; // Clear entire line
#endif
}
