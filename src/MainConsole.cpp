#include "MainConsole.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "Screen.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>

MainConsole::MainConsole(ConsoleManager& manager)
    : consoleManager(manager) {}

void MainConsole::run() {
    consoleManager.setCurrentPrompt("Main> ");
    std::string input;
    while (true) {
        consoleManager.printPrompt();
        std::getline(std::cin, input);

        if (input == "exit") {
            if (consoleManager.isInitialized()) {
                consoleManager.stopSchedulerTest();
                consoleManager.stopScheduler();
            }
            break;
        }
        else if (input == "clear") {
            system("CLS");
        }
        else {
            handleCommand(input);
        }
    }
}

void MainConsole::handleCommand(const std::string& input) {
    if (!consoleManager.isInitialized()) {
        if (input == "initialize") {
            if (!consoleManager.initialize()) {
                std::cout << "Failed to initialize system." << std::endl;
            }
            else {
                std::cout << "System initialized successfully with "
                    << "\"" << Config::getInstance().getSchedulerType() << "\""
                    << " scheduler." << std::endl;
            }
        }
        else if (input == "exit") {
            // Exit is handled in the main loop
        }
        else {
            std::cout << "System not initialized. Please run 'initialize' command first." << std::endl;
        }
        return;
    }

    std::istringstream iss(input);
    std::string command;
    iss >> command;

    if (command == "screen") {
        std::string flag, processName;
        iss >> flag >> processName;

        if (flag == "-s") {
            if (!processName.empty()) {
                consoleManager.createProcess(processName);
                std::cout << "Process '" << processName << "' created.\n";
            }
            else {
                std::cout << "Please specify a process name.\n";
            }
        }
        else if (flag == "-r") {
            if (!processName.empty()) {
                Process* process = consoleManager.getProcess(processName);
                if (process) {
                    consoleManager.switchToScreen(process);
                }
            }
            else {
                std::cout << "Please specify a process name to resume.\n";
            }
        }
        else if (flag == "-ls") {
            auto& processes = consoleManager.getProcesses();
            Scheduler* scheduler = consoleManager.getScheduler();
            auto runningProcesses = scheduler ? scheduler->getRunningProcesses() : std::map<Process*, int>();

            std::vector<Process*> running;
            std::vector<Process*> finished;

            for (const auto& pair : processes) {
                Process* process = pair.second;
                if (runningProcesses.find(process) != runningProcesses.end()) {
                    running.push_back(process);
                }
                else if (process->isCompleted()) {
                    finished.push_back(process);
                }
                else {
                    // Process is not running and not completed; ignoring for now
                }
            }

            // Display CPU utilization and core information
            if (scheduler) {
                int totalCores = scheduler->getTotalCores();
                int busyCores = scheduler->getBusyCores();
                int availableCores = totalCores - busyCores;
                double cpuUtilization = ((double)busyCores / totalCores) * 100.0;

                std::cout << "\nCPU utilization: " << std::fixed << std::setprecision(2) << cpuUtilization << "%\n";
                std::cout << "Cores used: " << busyCores << "\n";
                std::cout << "Cores available: " << availableCores << "\n";
            }
            else {
                std::cout << "\nScheduler is not initialized.\n";
            }

            std::cout << "\n-------------------------------------------\n";
            std::cout << "Running processes:\n";

            for (Process* process : running) {
                int coreId = runningProcesses[process];
                std::string processName = process->getName();
                std::time_t creationTime = process->getCreationTime();
                std::tm creationTm;

                localtime_s(&creationTm, &creationTime);

                char timeBuffer[30];
                std::strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y %I:%M:%S%p", &creationTm);
                std::string timeStr = timeBuffer;

                int currentLine = process->getCurrentLine();
                int totalLines = process->getTotalLines();

                // Format: processName  (creationTime)  Core: coreId   currentLine / totalLines
                std::cout << std::left << std::setw(15) << processName
                    << "(" << timeStr << ")    "
                    << "Core: " << coreId << "     "
                    << currentLine << " / " << totalLines << "\n";
            }

            std::cout << "\nFinished processes:\n";
            for (Process* process : finished) {
                std::string processName = process->getName();
                std::time_t creationTime = process->getCreationTime();
                std::tm creationTm;

                localtime_s(&creationTm, &creationTime);

                char timeBuffer[30];
                std::strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y %I:%M:%S%p", &creationTm);
                std::string timeStr = timeBuffer;

                int totalLines = process->getTotalLines();

                // Format: processName  (creationTime)  Finished   totalLines / totalLines
                std::cout << std::left << std::setw(15) << processName
                    << "(" << timeStr << ")    "
                    << "Finished     "
                    << totalLines << " / " << totalLines << "\n";
            }
            std::cout << "-------------------------------------------\n\n";
        }
        else {
            std::cout << "Invalid flag for screen command.\n";
            std::cout << "Usage:\n";
            std::cout << "  screen -s [process_name] : Start a new process\n";
            std::cout << "  screen -r [process_name] : Resume an existing process\n";
            std::cout << "  screen -ls               : List all processes\n";
        }
    }
    else if (command == "scheduler-pause") {
        consoleManager.pauseScheduler();
    }
    else if (command == "scheduler-resume") {
        consoleManager.resumeScheduler();
    }
    else if (command == "scheduler-10") {
        consoleManager.startScheduler10();
    }
    else if (command == "scheduler-test") {
        consoleManager.startSchedulerTest();
    }
    else if (command == "scheduler-stop") {
        consoleManager.stopSchedulerTest();
    }
    else if (command == "report-util") {
        std::ofstream logfile("csopesy-log.txt", std::ios::app);

        if (!logfile.is_open()) {
            std::cout << "Failed to open csopesy-log.txt for writing.\n";
            return;
        }

        std::time_t now = std::time(nullptr);
        char buffer[26];
        ctime_s(buffer, sizeof(buffer), &now);
        logfile << "Report generated at: " << buffer << "\n";

        auto& processes = consoleManager.getProcesses();
        Scheduler* scheduler = consoleManager.getScheduler();
        auto runningProcesses = scheduler ? scheduler->getRunningProcesses() : std::map<Process*, int>();

        std::vector<Process*> running;
        std::vector<Process*> finished;

        for (const auto& pair : processes) {
            Process* process = pair.second;
            if (runningProcesses.find(process) != runningProcesses.end()) {
                running.push_back(process);
            }
            else if (process->isCompleted()) {
                finished.push_back(process);
            }
            else {
                // Process is not running and not completed; ignoring for now
            }
        }

        // Display CPU utilization and core information
        if (scheduler) {
            int totalCores = scheduler->getTotalCores();
            int busyCores = scheduler->getBusyCores();
            int availableCores = totalCores - busyCores;
            double cpuUtilization = ((double)busyCores / totalCores) * 100.0;

            logfile << "CPU Utilization: " << std::fixed << std::setprecision(2) << cpuUtilization << "%\n";
            logfile << "Cores Used: " << busyCores << "\n";
            logfile << "Cores Available: " << availableCores << "\n";
        }
        else {
            logfile << "\nScheduler is not initialized.\n";
        }

        logfile << "\n-------------------------------------------\n";
        logfile << "Running processes:\n";

        for (Process* process : running) {
            int coreId = runningProcesses[process];
            std::string processName = process->getName();
            std::time_t creationTime = process->getCreationTime();
            std::tm creationTm;

            localtime_s(&creationTm, &creationTime);

            char timeBuffer[30];
            std::strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y %I:%M:%S%p", &creationTm);
            std::string timeStr = timeBuffer;

            int currentLine = process->getCurrentLine();
            int totalLines = process->getTotalLines();

            // Format: processName  (creationTime)  Core: coreId   currentLine / totalLines
            logfile << std::left << std::setw(15) << processName
                << "(" << timeStr << ")    "
                << "Core: " << coreId << "     "
                << currentLine << " / " << totalLines << "\n";
        }

        logfile << "\nFinished processes:\n";
        for (Process* process : finished) {
            std::string processName = process->getName();
            std::time_t creationTime = process->getCreationTime();
            std::tm creationTm;

            localtime_s(&creationTm, &creationTime);

            char timeBuffer[30];
            std::strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y %I:%M:%S%p", &creationTm);
            std::string timeStr = timeBuffer;

            int totalLines = process->getTotalLines();

            // Format: processName  (creationTime)  Finished   totalLines / totalLines
            logfile << std::left << std::setw(15) << processName
                << "(" << timeStr << ")    "
                << "Finished     "
                << totalLines << " / " << totalLines << "\n";
        }

        logfile << "-------------------------------------------\n\n\n";
        logfile.close();
        std::cout << "Utilization report saved to csopesy-log.txt.\n";
    }
    else if (command == "initialize") {
        std::cout << "System is already initialized.\n";
    }
    else {
        std::cout << "Command not recognized. Please try again.\n";
    }
}