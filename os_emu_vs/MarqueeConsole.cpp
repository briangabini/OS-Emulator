#include "MarqueeConsole.h"
#include <iostream>
#include <conio.h> // For _kbhit() and _getch()

MarqueeConsole::MarqueeConsole()
    : AConsole("MarqueeConsole"), marqueeText("Welcome to the Marquee Console! "), position(0), screenWidth(80), running(true) {
}

MarqueeConsole::~MarqueeConsole() {
    running = false;
    if (inputThread.joinable()) {
        inputThread.join();         // Wait for the input thread to finish, synchronize with main thread
    }
}

void MarqueeConsole::onEnabled() {
    inputThread = std::thread(&MarqueeConsole::handleInput, this);
    display();
    process();
}

void MarqueeConsole::process() {
    while (running) {
        updateMarquee();
        display();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust for refresh rate
    }
}

void MarqueeConsole::display() {
    clearScreen();
    std::string displayText = marqueeText.substr(position) + marqueeText.substr(0, position);
    std::cout << displayText.substr(0, screenWidth) << '\n';
    std::cout << "\n\n\n\n\n";
    std::cout << "Input: " << userInput << std::flush;
}

void MarqueeConsole::updateMarquee() {
    position = (position + 1) % marqueeText.length();
}

void MarqueeConsole::handleInput() {
    while (running) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 'q') {
                running = false;
                break;
            }
            else if (ch == '\b') { // Handle backspace
                if (!userInput.empty()) {
                    userInput.pop_back();
                }
            }
            else {
                userInput += static_cast<char>(ch);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust for input polling rate
    }
}

void MarqueeConsole::clearScreen() {
    // Clear the console screen
	system("cls");
}