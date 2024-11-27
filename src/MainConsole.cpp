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
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        std::cout << "No command entered.\n";
        return;
    }

    std::string command = tokens[0];

    if (command == "screen") {
        if (tokens.size() < 2) {
            std::cout << "Invalid usage of screen command.\n";
            return;
        }
        std::string flag = tokens[1];

        if (flag == "-s") {
            if (tokens.size() >= 3) {
                std::string processName = tokens[2];
                consoleManager.createProcess(processName);
                std::cout << "Process '" << processName << "' created.\n";
            }
            else {
                std::cout << "Please specify a process name.\n";
            }
        }
        else if (flag == "-r") {
            if (tokens.size() >= 3) {
                std::string processName = tokens[2];
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
            bool showAll = false;
            if (tokens.size() >= 3 && tokens[2] == "-a") {
                showAll = true;
            }

            Scheduler* scheduler = consoleManager.getScheduler();
            if (!scheduler) {
                std::cout << "Scheduler is not initialized.\n";
                return;
            }

            auto runningProcessesMap = scheduler->getRunningProcesses();
            auto queuedProcesses = scheduler->getQueuedProcesses();
            auto finishedProcesses = scheduler->getFinishedProcesses();

            std::vector<Process*> runningProcesses;
            for (const auto& pair : runningProcessesMap) {
                runningProcesses.push_back(pair.first);
            }

            // Display CPU utilization and core information
            int totalCores = scheduler->getTotalCores();
            int busyCores = scheduler->getBusyCores();
            int availableCores = totalCores - busyCores;
            double cpuUtilization = ((double)busyCores / totalCores) * 100.0;

            std::cout << "\nCPU utilization: " << std::fixed << std::setprecision(2) << cpuUtilization << "%\n";
            std::cout << "Cores used: " << busyCores << "\n";
            std::cout << "Cores available: " << availableCores << "\n";

            std::cout << "\n-------------------------------------------------------";

            if (showAll) {
                displayQueuedProcesses(queuedProcesses);
            }

            displayRunningProcesses(runningProcesses, runningProcessesMap);

            displayFinishedProcesses(finishedProcesses);

            std::cout << "-------------------------------------------------------\n\n";
        }
        else {
            std::cout << "Invalid flag for screen command.\n";
            std::cout << "Usage:\n";
            std::cout << "  screen -s [process_name]       : Start a new process\n";
            std::cout << "  screen -r [process_name]       : Resume an existing process\n";
            std::cout << "  screen -ls                     : List running and finished processes\n";
            std::cout << "  screen -ls -a                  : List all processes including queued\n";
        }
    }
    else if (command == "process-smi") {
        displayProcessSmi();
    }
    else if (command == "vmstat") {
        displayVmStat();
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
        reportUtil();
    }
    else if (command == "log") {
        if (tokens.size() >= 2) {
            std::string option = tokens[1];
            if (option == "on") {
                Process::setLoggingEnabled(true);
                std::cout << "Process logging enabled.\n";
            }
            else if (option == "off") {
                Process::setLoggingEnabled(false);
                std::cout << "Process logging disabled.\n";
            }
            else {
                std::cout << "Usage: log [on|off]\n";
            }
        }
        else {
            std::cout << "Usage: log [on|off]\n";
        }
    }
    else if (command == "initialize") {
        std::cout << "System is already initialized.\n";
    }
    else {
        std::cout << "Command not recognized. Please try again.\n";
    }
}

void MainConsole::displayProcessSmi() {
    MemoryManager& memoryManager = consoleManager.getMemoryManager();
    Scheduler* scheduler = consoleManager.getScheduler();
    if (!scheduler) {
        std::cout << "Scheduler is not initialized.\n";
        return;
    }

    std::cout << "\n+---------------------------------------------+\n";
    std::cout << "|          PROCESS-SMI V01.00 Monitor         |\n";
    std::cout << "+---------------------------------------------+\n";

    // System Utilization Section
    double cpuUtilization = ((double)scheduler->getBusyCores() / scheduler->getTotalCores()) * 100.0;
    unsigned int totalMem = memoryManager.getTotalMemory();
    unsigned int usedMem = memoryManager.getUsedMemory();
    double memUtil = memoryManager.getMemoryUtilization();

    std::cout << "| System Utilization                          |\n";
    std::cout << "| CPU Usage    : " << std::setw(9) << std::fixed << std::setprecision(1)
        << cpuUtilization << "%" << std::string(19, ' ') << "|\n";
    std::cout << "| Memory       : " << std::setw(7) << usedMem << " KB / "
        << totalMem << " KB" << std::string(9, ' ') << "|\n";
    std::cout << "| Memory Usage : " << std::setw(9) << std::fixed << std::setprecision(1)
        << memUtil << "%" << std::string(19, ' ') << "|\n";

    // Process Memory Section
    std::cout << "+---------------------------------------------+\n";
    std::cout << "|   PID   Process Name          Memory Usage  |\n";
    std::cout << "|---------------------------------------------|\n";

    auto processesInMemory = memoryManager.getProcessesInMemory();
    for (const auto& entry : processesInMemory) {
        Process* process = entry.first;
        std::string processName = process->getName();
        if (processName.length() > 16) {
            processName = processName.substr(0, 13) + "...";
        }
        std::cout << "| " << std::setw(5) << process->getId() << "   "
            << std::left << std::setw(16) << processName << " "
            << std::right << std::setw(9) << entry.second << " KB"
            << std::string(7, ' ') << "|\n";
    }

    if (processesInMemory.empty()) {
        std::cout << "| No processes currently in memory"
            << std::string(12, ' ') << "|\n";
    }

    std::cout << "+---------------------------------------------+\n\n";
}

void MainConsole::displayVmStat() {
    MemoryManager& memoryManager = consoleManager.getMemoryManager();

    std::cout << "\n+--------------------------------+\n";
    std::cout << "|      Virtual Memory Stats      |\n";
    std::cout << "+--------------------------------+\n";
    std::cout << "| Memory Summary:                |\n";
    std::cout << "| Total Memory  : " << std::setw(10) << memoryManager.getTotalMemory()
        << " KB" << std::string(2, ' ') << "|\n";
    std::cout << "| Used Memory   : " << std::setw(10) << memoryManager.getUsedMemory()
        << " KB" << std::string(2, ' ') << "|\n";
    std::cout << "| Free Memory   : " << std::setw(10) << memoryManager.getFreeMemory()
        << " KB" << std::string(2, ' ') << "|\n";

    std::cout << "+--------------------------------+\n";
    std::cout << "| CPU Statistics:                |\n";
    std::cout << "| Idle Ticks    : " << std::setw(13) << memoryManager.getIdleCpuTicks()
        << std::string(2, ' ') << "|\n";
    std::cout << "| Active Ticks  : " << std::setw(13) << memoryManager.getActiveCpuTicks()
        << std::string(2, ' ') << "|\n";
    std::cout << "| Total Ticks   : " << std::setw(13) << memoryManager.getTotalCpuTicks()
        << std::string(2, ' ') << "|\n";

    if (memoryManager.isPaging()) {
        std::cout << "+--------------------------------+\n";
        std::cout << "| Paging Information:            |\n";
        std::cout << "| Pages In      : " << std::setw(13) << memoryManager.getNumPagedIn()
            << std::string(2, ' ') << "|\n";
        std::cout << "| Pages Out     : " << std::setw(13) << memoryManager.getNumPagedOut()
            << std::string(2, ' ') << "|\n";
    }

    std::cout << "+--------------------------------+\n\n";
}

void MainConsole::displayRunningProcesses(const std::vector<Process*>& runningProcesses, const std::map<Process*, int>& runningProcessesMap) {
    if (runningProcesses.empty()) {
        std::cout << "\nNo running processes.\n";
        return;
    }
    std::cout << "\nRunning processes:\n";

    for (Process* process : runningProcesses) {
        int coreId = runningProcessesMap.at(process);
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
}

void MainConsole::displayFinishedProcesses(const std::vector<Process*>& finishedProcesses) {
    if (finishedProcesses.empty()) {
        std::cout << "\nNo finished processes.\n";
        return;
    }
    std::cout << "\nFinished processes:\n";
    for (Process* process : finishedProcesses) {
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
}

void MainConsole::displayQueuedProcesses(const std::vector<Process*>& queuedProcesses) {
    if (queuedProcesses.empty()) {
        std::cout << "\nNo queued processes.\n";
        return;
    }
    std::cout << "\nQueued processes:\n";
    for (Process* process : queuedProcesses) {
        std::string processName = process->getName();
        std::time_t creationTime = process->getCreationTime();
        std::tm creationTm;

        localtime_s(&creationTm, &creationTime);

        char timeBuffer[30];
        std::strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y %I:%M:%S%p", &creationTm);
        std::string timeStr = timeBuffer;

        int currentLine = process->getCurrentLine();
        int totalLines = process->getTotalLines();

        // Format: processName  (creationTime)  Queued   currentLine / totalLines
        std::cout << std::left << std::setw(15) << processName
            << "(" << timeStr << ")    "
            << "Queued     "
            << currentLine << " / " << totalLines << "\n";
    }
}

void MainConsole::reportUtil() {
    Scheduler* scheduler = consoleManager.getScheduler();
    if (!scheduler) {
        std::cout << "Scheduler is not initialized.\n";
        return;
    }

    auto runningProcessesMap = scheduler->getRunningProcesses();
    auto queuedProcesses = scheduler->getQueuedProcesses();
    auto finishedProcesses = scheduler->getFinishedProcesses();

    std::vector<Process*> runningProcesses;
    for (const auto& pair : runningProcessesMap) {
        runningProcesses.push_back(pair.first);
    }

    std::ofstream logfile("csopesy-log.txt", std::ios::app);

    if (!logfile.is_open()) {
        std::cout << "Failed to open csopesy-log.txt for writing.\n";
        return;
    }

    std::time_t now = std::time(nullptr);
    char buffer[26];
    ctime_s(buffer, sizeof(buffer), &now);
    logfile << "Report generated at: " << buffer << "\n";

    // Display CPU utilization and core information
    int totalCores = scheduler->getTotalCores();
    int busyCores = scheduler->getBusyCores();
    int availableCores = totalCores - busyCores;
    double cpuUtilization = ((double)busyCores / totalCores) * 100.0;

    logfile << "CPU utilization: " << std::fixed << std::setprecision(2) << cpuUtilization << "%\n";
    logfile << "Cores used: " << busyCores << "\n";
    logfile << "Cores available: " << availableCores << "\n";

    logfile << "\n-------------------------------------------------------";

    // Display queued processes
    if (!queuedProcesses.empty()) {
        logfile << "\nQueued processes:\n";
        for (Process* process : queuedProcesses) {
            std::string processName = process->getName();
            std::time_t creationTime = process->getCreationTime();
            std::tm creationTm;

            localtime_s(&creationTm, &creationTime);

            char timeBuffer[30];
            std::strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y %I:%M:%S%p", &creationTm);
            std::string timeStr = timeBuffer;

            int currentLine = process->getCurrentLine();
            int totalLines = process->getTotalLines();

            // Format: processName  (creationTime)  Queued   currentLine / totalLines
            logfile << std::left << std::setw(15) << processName
                << "(" << timeStr << ")    "
                << "Queued     "
                << currentLine << " / " << totalLines << "\n";
        }
    }
    else {
        logfile << "\nNo queued processes.\n";
    }

    // Display running processes
    if (!runningProcesses.empty()) {
        logfile << "\nRunning processes:\n";
        for (Process* process : runningProcesses) {
            int coreId = runningProcessesMap.at(process);
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
    }
    else {
        logfile << "\nNo running processes.\n";
    }

    // Display finished processes
    if (!finishedProcesses.empty()) {
        logfile << "\nFinished processes:\n";
        for (Process* process : finishedProcesses) {
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
    }
    else {
        logfile << "\nNo finished processes.\n";
    }

    logfile << "-------------------------------------------------------\n\n\n";
    logfile.close();
    std::cout << "Utilization report saved to csopesy-log.txt.\n";
}