#include "Screen.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "Command.h"
#include <iostream>
#include <sstream>
#include <ctime>

#ifdef _WIN32
#define CLEAR_COMMAND "CLS"
#else
#define CLEAR_COMMAND "clear"
#endif

Screen::Screen(ConsoleManager& manager, Process* process)
    : consoleManager(manager), process(process) {}

void Screen::run() {
    std::string input;
    while (true) {
        displayProcessScreen();
        std::cout << process->getName() << "> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }
        else if (input == "clear") {
            system(CLEAR_COMMAND);
        }
        else {
            // Handle commands
            std::istringstream iss(input);
            std::string command;
            iss >> command;
            if (command == "print") {
                std::string message;
                std::getline(iss, message);
                // Remove leading spaces
                if (!message.empty() && message[0] == ' ') {
                    message.erase(0, 1);
                }
                // Create PrintCommand and add to process
                process->addCommand(new PrintCommand(message));
                std::cout << "Print command added to process.\n";
            }
            else {
                std::cout << "Unknown command: " << command << std::endl;
            }
        }
    }
}

void Screen::displayProcessScreen() {
    system(CLEAR_COMMAND);
    std::cout << "Process Name: " << process->getName() << "\n";

    std::time_t creationTime = process->getCreationTime();
    char buffer[26]; // Buffer for ctime

#ifdef _WIN32
    ctime_s(buffer, sizeof(buffer), &creationTime);
#else
    ctime_r(&creationTime, buffer);
#endif

    std::cout << "Creation Time: " << buffer;

    std::cout << "Current Line Executed: " << process->getCurrentLine()
        << "/" << process->getTotalLines() << "\n";
    if (!process->isCompleted()) {
        std::cout << "Executing: " << process->getCurrentCodeLine() << "\n";
    }
    else {
        std::cout << "Process has completed execution.\n";
    }
}
