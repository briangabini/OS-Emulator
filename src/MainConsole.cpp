#include "MainConsole.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "Screen.h"
#include "Marquee.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

#ifdef _WIN32
#define CLEAR_COMMAND "CLS"
#else
#define CLEAR_COMMAND "clear"
#endif

MainConsole::MainConsole(ConsoleManager& manager)
    : consoleManager(manager) {}

void MainConsole::run() {
    std::string input;
    while (true) {
        std::cout << "Main> ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }
        else if (input == "clear") {
            system(CLEAR_COMMAND);
        }
        else {
            handleCommand(input);
        }
    }
}

void MainConsole::handleCommand(const std::string& input) {
    std::istringstream iss(input);
    std::string command;
    iss >> command;

    if (command == "screen") {
        std::string flag, processName;
        iss >> flag >> processName;

        if (flag == "-s") {
            consoleManager.createProcess(processName);
        }
        else if (flag == "-r") {
            Process* process = consoleManager.getProcess(processName);
            if (process) {
                consoleManager.switchToScreen(process);
            }
        }
        else {
            std::cout << "Invalid flag for screen command.\n";
        }
    }
    else if (command == "scheduler-test") {
        std::cout << "Command 'scheduler-test' recognized. Doing something.\n";
    }
    else if (command == "scheduler-stop") {
        std::cout << "Command 'scheduler-stop' recognized. Doing something.\n";
    }
    else if (command == "report-util") {
        std::cout << "Command 'report-util' recognized. Doing something.\n";
    }
    else if (command == "initialize") {
        std::cout << "Command 'initialize' recognized. Doing something.\n";
    }
    else if (command == "nvidia-smi") {
        displayNvidiaSmi();
    }
    else if (command == "marquee") {
        consoleManager.switchToMarquee();
    }
    else if (command == "marqueent") {
        consoleManager.switchToMarqueeNT();
    }
    else {
        std::cout << "Command not recognized. Please try again.\n";
    }
}

void MainConsole::displayNvidiaSmi() {
    // Clear the console
    system(CLEAR_COMMAND);

    // Get current time
    std::time_t t = std::time(nullptr);
    std::tm localTime;

    errno_t err = localtime_s(&localTime, &t);
    if (err != 0) {
        std::cout << "Error getting local time.\n";
        return;
    }

    // Print current time
    std::cout << std::put_time(&localTime, "%a %b %e %H:%M:%S %Y") << "\n";

    // Print GPU summary
    std::cout << "+---------------------------------------------------------------------------------------+\n";
    std::cout << "| NVIDIA-SMI 546.17                 Driver Version: 546.17       CUDA Version: 12.3     |\n";
    std::cout << "|-----------------------------------------+----------------------+----------------------+\n";
    std::cout << "| GPU  Name                     TCC/WDDM  | Bus-Id        Disp.A | Volatile Uncorr. ECC |\n";
    std::cout << "| Fan  Temp   Perf          Pwr:Usage/Cap |         Memory-Usage | GPU-Util  Compute M. |\n";
    std::cout << "|                                         |                      |               MIG M. |\n";
    std::cout << "|=========================================+======================+======================|\n";
    std::cout << "|   0  NVIDIA GeForce RTX 2070 ...  WDDM  | 00000000:09:00.0  On |                  N/A |\n";
    std::cout << "|  0%   38C    P8              19W / 215W |    517MiB /  8192MiB |      1%      Default |\n";
    std::cout << "|                                         |                      |                  N/A |\n";
    std::cout << "+-----------------------------------------+----------------------+----------------------+\n\n";

    // Print Processes header
    std::cout << "+---------------------------------------------------------------------------------------+\n";
    std::cout << "| Processes:                                                                            |\n";
    std::cout << "|  GPU   GI   CI        PID   Type   Process name                            GPU Memory |\n";
    std::cout << "|        ID   ID                                                             Usage      |\n";
    std::cout << "|=======================================================================================|\n";

    // Dummy process data
    struct ProcessInfo {
        int gpu;
        std::string gi;
        std::string ci;
        int pid;
        std::string type;
        std::string processName;
        std::string gpuMemoryUsage;
    };

    ProcessInfo processes[5] = {
        {0, "N/A", "N/A", 1234, "C+G", "ThisIsAProcess.exe", "N/A"},
        {0, "N/A", "N/A", 5678, "C+G", "CS2.exe", "N/A"},
        {0, "N/A", "N/A", 98012, "C+G", "VeryLongProcessNameThatExceedsColumnWidth.exe", "N/A"},
        {0, "N/A", "N/A", 3456, "C+G", "VisualStudio.exe", "N/A"},
        {0, "N/A", "N/A", 7890, "C+G", "CallofDutyInfiniteWarfare.exe", "N/A"}
    };

    // Print each process
    for (const auto& proc : processes) {
        std::cout << "|   ";
        std::cout << std::setw(2) << proc.gpu << "   ";
        std::cout << std::setw(3) << proc.gi << "  ";
        std::cout << std::setw(3) << proc.ci << "  ";
        std::cout << std::setw(8) << proc.pid << "    ";
        std::cout << std::setw(3) << proc.type << "   ";

        // Process name needs special handling for width
        std::string procName = proc.processName;
        if (procName.length() > 37) {
            procName = procName.substr(0, 34) + "...";
        }
        std::cout << std::left << std::setw(37) << procName << std::right;

        std::cout << std::setw(8) << proc.gpuMemoryUsage << "      |\n";
    }

    std::cout << "+---------------------------------------------------------------------------------------+\n";
}