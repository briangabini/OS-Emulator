// os_emu_vs.cpp
#include "Screen.h"
#include "Util.h"
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <sstream>

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

// Command handler for all user inputs
void onEvent(const std::string_view command, Screen& screen) {
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

        if (flag == "-s") {
            screen.createScreen(processName);
        }
        else if (flag == "-r") {
            screen.reattachScreen(processName);
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
    Screen screen;

    Util::greetings();

    while (true) {
        getUserInput(userInput);
        onEvent(userInput, screen);
    }

    return 0;
}
