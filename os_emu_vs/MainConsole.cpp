#include "MainConsole.h"
#include <iostream>
#include <array>
#include <vector>
#include <sstream>
#include "BaseScreen.h"
#include "ConsoleManager.h"

#define windows

namespace {
	inline constexpr char asciiHeader[] =
		"  ______     _______.  ______   .______    _______     _______.____    ____ \n"
		" /      |   /       | /  __  \\  |   _  \\  |   ____|   /       |\\   \\  /   / \n"
		"|  ,----'  |   (----`|  |  |  | |  |_)  | |  |__     |   (----` \\   \\/   /  \n"
		"|  |        \\   \\    |  |  |  | |   ___/  |   __|     \\   \\      \\_    _/   \n"
		"|  `----.----)   |   |  `--'  | |  |      |  |____.----)   |       |  |     \n"
		" \\______|_______/     \\______/  | _|      |_______|_______/        |__|     \n"
		"                                                                            \n";

	inline constexpr int commandsCount = 7;

	inline constexpr std::array<std::string_view, commandsCount> commands = {
		"initialize",
		"screen",
		"scheduler-test",
		"scheduler-stop",
		"report-util",
		"clear",
		"exit"
	};

	inline bool checkIfCommandExists(const std::string_view command) {
		return std::ranges::find_if(commands, [&command](std::string_view cmd) {
			return command == cmd;
			}) != commands.end();
	}

	inline void clearScreen() {
		std::cout << std::flush;

	#ifdef windows
		std::system("cls");
	#else
		std::system("clear");
	#endif
	}
}

namespace {
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

	void onEvent(const std::string_view command) {
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
				const auto newProcess = std::make_shared<Process>(processName);
				const auto newBaseScreen = std::make_shared<BaseScreen>(newProcess, processName);

				try {
					ConsoleManager::getInstance()->registerScreen(newBaseScreen);
					ConsoleManager::getInstance()->switchToScreen(processName);
				}
				catch (const std::exception& e) {
					return;
				}
			}
			else if (flag == "-r") {
				ConsoleManager::getInstance()->switchToScreen(processName);
			}
			else {
				std::cout << "Error: Invalid screen command flag.\n";
				std::cout << "Usage: screen -r <process_name> or screen -s <process_name>\n";
			}
			return;
		}

		if (!checkIfCommandExists(command)) {
			std::cout << command << " command not recognized. Doing nothing.\n";
			return;
		}

		std::cout << command << " command recognized. Doing something.\n";

		if (command == "clear") {
			clearScreen();
		}
		else if (command == "exit") {
			exit(0);
		}
	}

	void getUserInput(std::string& userInput) {
		std::cout << "Enter a command: ";
		std::getline(std::cin >> std::ws, userInput);
	}
}


void MainConsole::onEnabled() {
	display();
	process();
}

void MainConsole::process() {
	std::string userInput;
	while (true) {
		getUserInput(userInput);
		onEvent(userInput);
	}
}

void MainConsole::display() {
	std::cout << asciiHeader << '\n';
	std::cout << "\033[32m" << "Hello, Welcome to CSOPESY commandline!\n";
	std::cout << "\033[93m" << "Type \'exit\' to quit, \'clear\' to clear the screen\n";
	std::cout << "\033[0m"; // reset color
}