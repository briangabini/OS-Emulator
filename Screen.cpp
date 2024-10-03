#include "Screen.h"
#include "ConsoleManager.h"
#include <iostream>
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
        std::cout << "Process> ";
        std::getline(std::cin, input);
        if (input == "exit") {
            break;
        }
        else {
            std::cout << "Command not recognized in process screen.\n";
        }
    }
}

void Screen::displayProcessScreen() {
    system(CLEAR_COMMAND);
    std::cout << "Process Name: " << process->getName() << "\n";

    std::time_t creationTime = process->getCreationTime();
    char buffer[26]; // ctime_s expects a buffer of at least 26 characters
    errno_t err = ctime_s(buffer, sizeof(buffer), &creationTime);
    if (err == 0) {
        std::cout << "Creation Time: " << buffer;
    }
    else {
        std::cout << "Error formatting creation time.\n";
    }

    std::cout << "Current Line Executed: " << process->getCurrentLine()
        << "/" << process->getTotalLines() << "\n";
    if (!process->isCompleted()) {
        std::cout << "Executing: " << process->getCurrentCodeLine() << "\n";
    }
    else {
        std::cout << "Process has completed execution.\n";
    }
}
