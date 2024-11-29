#include "Screen.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "PrintCommand.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

Screen::Screen(ConsoleManager& manager, Process* process)
    : consoleManager(manager), process(process) {}

void Screen::run() {
    displayProcessScreen();

    std::string input;
    while (true) {
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
            else if (command == "process-smi") {
                displayProcessScreen();
            }
            else {
                std::cout << "Unknown command: " << command << std::endl;
            }
        }
    }
}

void Screen::displayProcessScreen() {
    std::string processName = process->getName();
    int processId = process->getId();
    std::time_t creationTime = process->getCreationTime();
    int currentLine = process->getCurrentLine();
    int totalLines = process->getTotalLines();

    double progress = 0.0;
    if (totalLines > 0) {
        progress = ((double)currentLine / totalLines) * 100.0;
        if (progress > 100.0) progress = 100.0;
    }

    std::string status;
    if (process->isCompleted()) {
        status = "Completed";
    }
    else {
        Scheduler* scheduler = consoleManager.getScheduler();
        bool isRunning = false;
        if (scheduler) {
            auto runningProcesses = scheduler->getRunningProcesses();
            if (runningProcesses.find(process) != runningProcesses.end()) {
                isRunning = true;
            }
        }

        if (isRunning) {
            status = "Executing";
        }
        else if (currentLine == 0) {
            status = "Not started";
        }
        else {
            status = "Paused";
        }
    }

    char buffer[26];
    ctime_s(buffer, sizeof(buffer), &creationTime);
    std::string creationTimeStr = buffer;
    if (!creationTimeStr.empty() && creationTimeStr.back() == '\n') {
        creationTimeStr.pop_back();
    }

    // Display process information
    std::cout << "Process: " << processName << "\n";
    std::cout << "ID: " << processId << "\n";
    std::cout << "Creation Time: " << creationTimeStr << "\n\n";

    if (process->isCompleted()) {
        // Output format for completed process
        std::cout << "Progress: " << std::fixed << std::setprecision(2) << progress << "% "
            << "(" << currentLine << " / " << totalLines << ")\n";
        std::cout << "Status: " << status << "\n\n";
    }
    else {
        // Output format for ongoing process
        std::cout << "Current instruction line: " << currentLine + 1 << "\n";
        std::cout << "Lines of code: " << totalLines << "\n";
        std::cout << "Progress: " << std::fixed << std::setprecision(2) << progress << "%\n";
        std::cout << "Status: " << status << "\n\n";
    }
}
