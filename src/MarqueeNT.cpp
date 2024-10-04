#include "MarqueeNT.h"
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

MarqueeNT::MarqueeNT(ConsoleManager& manager)
    : running(true), consoleManager(manager) {
    // Initialize messages for multiple lines
    messages = {
        R"(  _____   _____    _____   _____    _______   _____   __    __          _____   _____    _____   _____    _______   _____   __    __)",
        R"( / ____| / ____|  /  __ \  |  __ \  |  ____| / ____|  \ \  / /         / ____| / ____|  /  __ \  |  __ \  |  ____| / ____|  \ \  / /)",
        R"(| |      | (___   | |  | | | |__) | | |__    | (___    \ \/ /         | |      | (___   | |  | | | |__) | | |__    | (___    \ \/ / )",
        R"(| |       \___ \  | |  | | |  ___/  |  __|    \___ \    |  |          | |       \___ \  | |  | | |  ___/  |  __|    \___ \    |  |  )",
        R"(| |____   ____) | | |__| | | |      | |____   ____) |   |  |          | |____   ____) | | |__| | | |      | |____   ____) |   |  |  )",
        R"( \_____| |_____/  \_____/  |_|      |______| |_____/    |__|           \_____| |_____/  \_____/  |_|      |______| |_____/    |__|  )"
    };
}

MarqueeNT::~MarqueeNT() {
    running = false;
}

void MarqueeNT::run() {
    const int MARQUEE_WIDTH = 100;
    const size_t NUM_LINES = messages.size();
    std::vector<size_t> start_positions(NUM_LINES, 0);
    std::vector<size_t> msgLengths(NUM_LINES);

    for (size_t i = 0; i < NUM_LINES; ++i) {
        messages[i] += "        "; // Spaces in between messages
        msgLengths[i] = messages[i].length();
    }

    std::string user_input;
    size_t input_length = 0; // Tracks the number of characters in user_input

    // Move cursor below the marquee lines and display the prompt
    {
        std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD inputPosition = { 0, (SHORT)(NUM_LINES + 1) }; // Below the marquee lines
        SetConsoleCursorPosition(hConsole, inputPosition);
#else
        std::cout << "\033[" << (NUM_LINES + 2) << ";1H"; // Move cursor to line NUM_LINES+2, column 1
#endif
        std::cout << "Marquee> ";
        std::cout.flush();
    }

#ifdef _WIN32
    // Windows implementation using _kbhit() and _getch()
    while (running) {
        auto loop_start = std::chrono::steady_clock::now();

        // Handle input
        while (_kbhit()) {
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
                input_length = 0;
            }
            else if (ch == '\b' || ch == 127) { // Backspace key
                if (input_length > 0) {
                    user_input.pop_back();
                    input_length--;
                    {
                        std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                        std::cout << "\b \b";
                        std::cout.flush();
                    }
                }
                // If input_length is zero, do nothing (prevent backspacing over the prompt)
            }
            else {
                user_input += ch;
                input_length++;
                {
                    std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                    std::cout << ch;
                    std::cout.flush();
                }
            }
        }

        // Update marquee
        {
            std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());

            // Save cursor position
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            GetConsoleScreenBufferInfo(hConsole, &csbi);
            COORD originalCursorPosition = csbi.dwCursorPosition;

            // Update each marquee line
            for (size_t line = 0; line < NUM_LINES; ++line) {
                // Move cursor to the correct line
                COORD marqueePosition = { 0, (SHORT)line };
                SetConsoleCursorPosition(hConsole, marqueePosition);

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
            SetConsoleCursorPosition(hConsole, originalCursorPosition);
        }

        // Update start positions for scrolling
        for (size_t i = 0; i < NUM_LINES; ++i) {
            start_positions[i] = (start_positions[i] + 1) % msgLengths[i];
        }

        // Control loop timing
        auto loop_end = std::chrono::steady_clock::now();
        auto loop_duration = std::chrono::duration_cast<std::chrono::microseconds>(loop_end - loop_start);
        int sleep_time = 8333 - (int)loop_duration.count(); // Target 120Hz

        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
        }
    }

    // Exiting message
    {
        std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
        std::cout << "\nExiting the Marquee Console..." << std::endl;
        std::cout.flush();
    }

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

    size_t input_length = 0; // Tracks the number of characters in user_input

    while (running) {
        auto loop_start = std::chrono::steady_clock::now();

        // Handle input
        char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        while (n > 0) {
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
                input_length = 0;
            }
            else if (ch == 127 || ch == '\b') { // Backspace key
                if (input_length > 0) {
                    user_input.pop_back();
                    input_length--;
                    {
                        std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                        std::cout << "\b \b";
                        std::cout.flush();
                    }
                }
                // If input_length is zero, do nothing (prevent backspacing over the prompt)
            }
            else {
                user_input += ch;
                input_length++;
                {
                    std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
                    std::cout << ch;
                    std::cout.flush();
                }
            }
            n = read(STDIN_FILENO, &ch, 1);
        }

        // Update marquee
        {
            std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());

            // Save cursor position
            std::cout << "\033[s"; // Save cursor position

            // Update each marquee line
            for (size_t line = 0; line < NUM_LINES; ++line) {
                // Move cursor to the correct line
                std::cout << "\033[" << (line + 1) << ";1H"; // Move cursor to line (line + 1), column 1

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
            std::cout << "\033[u"; // Restore cursor position
        }

        // Update start positions for scrolling
        for (size_t i = 0; i < NUM_LINES; ++i) {
            start_positions[i] = (start_positions[i] + 1) % msgLengths[i];
        }

        // Control loop timing
        auto loop_end = std::chrono::steady_clock::now();
        auto loop_duration = std::chrono::duration_cast<std::chrono::microseconds>(loop_end - loop_start);
        int sleep_time = 8333 - (int)loop_duration.count(); // Target 120Hz

        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
        }
    }

    // Exiting message
    {
        std::lock_guard<std::mutex> lock(consoleManager.getIOMutex());
        std::cout << "\nExiting the Marquee Console..." << std::endl;
        std::cout.flush();
    }

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
#endif
}

void MarqueeNT::clearMarqueeLines() {
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
