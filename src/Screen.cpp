#include "Screen.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "PrintCommand.h"
#include <iostream>
#include <sstream>
#include <ctime>

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
            system("CLS");
        }
        else {
            std::istringstream iss(input);
            std::string command;
            iss >> command;
            if (command == "print") {
                std::string message;
                std::getline(iss, message);
                
                if (!message.empty() && message[0] == ' ') {
                    message.erase(0, 1);
                }
                
                process->addCommand(new PrintCommand(message));
                std::cout << "Print command added to process.\n";

                // Reset completed status and reschedule if a command is added when process is already finished
                if (process->isCompleted()) {
                    process->resetCompleted();
                    consoleManager.getScheduler()->addProcess(process);
                }
            }
            else {
                std::cout << "Unknown command: " << command << std::endl;
            }
        }
    }
}

void Screen::displayProcessScreen() {
    system("CLS");
    std::cout << "Process Name: " << process->getName() << "\n";

    std::time_t creationTime = process->getCreationTime();
    char buffer[26];

    ctime_s(buffer, sizeof(buffer), &creationTime);

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
