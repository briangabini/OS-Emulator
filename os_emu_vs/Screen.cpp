// Screen.cpp
#include "Screen.h"
#include <iostream>

void Screen::createScreen(const std::string& processName) {
    if (screens.contains(processName)) {
        std::cout << "Error: Screen for process " << processName << " already exists.\n";
        return;
    }

    screens[processName] = Process(processName);
    handleScreenCommand(processName);
}

void Screen::reattachScreen(const std::string& processName) {
    if (!screens.contains(processName)) {
        std::cout << "Error: Screen for process " << processName << " not found.\n";
        return;
    }

    handleScreenCommand(processName);
}

void Screen::handleScreenCommand(const std::string& processName) {
    const Process& process = screens[processName];

    std::cout << "\033[32m" << "Screen - Process: " << process.getName() << "\n";
    std::cout << "Instruction: " << process.getCurrentLine() << " / " << process.getTotalLines() << "\n";
    std::cout << "Created at: " << process.getTimestamp() << "\n";
    std::cout << "\033[0m";

    while (true) {
        std::string screenInput;
        std::cout << process.getName() << "> ";
        std::getline(std::cin >> std::ws, screenInput);

        if (screenInput == "exit") {
            std::cout << "Returning to main menu...\n";
            break;
        }

        std::cout << "Invalid command in screen. Type 'exit' to return.\n";
    }
}