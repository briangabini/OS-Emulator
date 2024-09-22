#include "os_emu_vs.h"
#include "util.h"
#include <iostream>
#include <string>
#include <string_view>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

#define _WIN32

// Get current timestamp in MM/DD/YYYY, HH:MM:SS AM/PM format
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;

#ifdef _WIN32
    // Windows: use localtime_s
    localtime_s(&local_tm, &now_time);
#else
    // Unix-like systems: use localtime_r
    localtime_r(&now_time, &local_tm);
#endif

    // Create a formatted string using std::put_time
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p");
    return oss.str();
}

// Helper function to split command into tokens
std::vector<std::string> splitCommand(const std::string& command) {
    std::istringstream iss(command);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Simulate entering a "screen" and displaying its process info
void handleScreenCommand(const std::string_view command, const std::string_view processName) {
    std::string_view timestamp = getCurrentTimestamp();

    std::cout << "\033[32m" << "Screen - Process: " << processName << "\n";
    std::cout << "Instruction: 1 / 100\n"; // Placeholder values for now
    std::cout << "Created at: " << timestamp << "\n";
    std::cout << "\033[0m";

    while (true) {
        std::string screenInput;
        std::cout << processName << "> ";
        std::getline(std::cin >> std::ws, screenInput);

        if (screenInput == "exit") {
            std::cout << "Returning to main menu...\n";
            break;
        }

    	std::cout << "Invalid command in screen. Type 'exit' to return.\n";
    }
}

// Command handler for all user inputs
void onEvent(const std::string_view command) {
    // Split the command into tokens
    std::vector<std::string> tokens = splitCommand(std::string(command));

    if (tokens.empty()) {
        std::cout << "Invalid command.\n";
        return;
    }

    if (tokens[0] == "screen") {
        if (tokens.size() < 3) {
            std::cout << "Error: Insufficient arguments for screen command.\n";
            std::cout << "Usage: screen -r <process_name> or screen -s <process_name>\n";
            return;
        }

        const std::string& flag = tokens[1];
        const std::string& processName = tokens[2];

        if (flag == "-r" || flag == "-s") {
            handleScreenCommand(command, processName);
        }
        else {
            std::cout << "Error: Invalid screen command flag.\n";
            std::cout << "Usage: screen -r <process_name> or screen -s <process_name>\n";
        }
        return;
    }

    if (!Util::checkIfCommandExists(command)) {
        std::cout << command << " command not recognized. Doing nothing.\n";
        return;
    }

    std::cout << command << " command recognized. Doing something.\n";

    if (command == "clear") {
        Util::clearScreen();
    }
    else if (command == "exit") {
        exit(0);
    }
}

// Prompt user for input
void getUserInput(std::string& userInput) {
    std::cout << "Enter a command: ";
    std::getline(std::cin >> std::ws, userInput);
}

// Main loop of the program
int main() {
    std::string userInput{};

    Util::greetings();

    while (true) {
        getUserInput(userInput);

        onEvent(userInput);
    }

    return 0;
}
