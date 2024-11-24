
#include "BaseScreen.h"
#include "ConsoleManager.h"
#include "GlobalConfig.h"
#include "GlobalScheduler.h"
#include "MemoryManager.h"
#include "MainConsole.h"
#include "TypedefRepo.h"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>


#include "FlatMemoryAllocator.h"
#include "os_emu_vs.h"

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

	inline constexpr int commandsCount = 9;

	inline constexpr std::array<std::string_view, commandsCount> commands = {
		"initialize",
		"screen",
		"scheduler-test",
		"scheduler-stop",
		"report-util",
		"clear",
		"exit",
		//"nvidia-smi,
		"marquee"
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

namespace MainConsoleUtil {
	// forward declarations

	// Helper function to split command into tokens
	std::vector<std::string> splitCommand(const std::string& command) {
		std::istringstream iss(command);
		std::vector<String> tokens;
		String token;
		while (iss >> token) {
			tokens.push_back(token);
		}
		return tokens;
	}
	bool initialized = false;

	void onEvent(const std::string_view command) {
		std::vector<String> tokens = splitCommand(String(command));

		if (tokens.empty()) {
			std::cout << "Invalid command.\n";
			return;
		}

		String processName{};

		if (tokens[0] == "screen") {
			if (tokens.size() < 2) {
				std::cout << "Error: Insufficient arguments for screen command.\n";
				std::cout << "Usage: screen -r <process_name> or screen -s <process_name>\n";
				return;
			}

			const String& flag = tokens[1];

			if (tokens.size() == 3)
				processName = tokens[2];

			if (flag == "-s") {
				if (tokens.size() < 3) {
					std::cout << "Error: Insufficient arguments for screen command.\n";
					std::cout << "Usage: screen -s <process_name>\n";
					return;
				}

				const auto newProcess = GlobalScheduler::getInstance()->createProcess(processName, Mode::USER);
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
				if (tokens.size() < 3) {
					std::cout << "Error: Insufficient arguments for screen command.\n";
					std::cout << "Usage: screen -r <process_name>\n";
					return;
				}

				ConsoleManager::getInstance()->switchToScreen(processName);
			}
			else if (flag == "-ls")
			{
				GlobalScheduler::getInstance()->monitorProcesses();
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

		if (command == "initialize") {
			GlobalConfig::initialize();
			GlobalConfig::getInstance()->loadConfigFromFile("config.txt");
			GlobalScheduler::initialize();
			MemoryManager::initialize();
			FlatMemoryAllocator::initialize();
		}
		else if (command == "clear") {
			clearScreen();
		}
		else if (command == "nvidia-smi") {
			//printNvidiaSmiOutput();
		}
		else if (command == "marquee")
		{
			ConsoleManager::getInstance()->switchConsole(MARQUEE_CONSOLE);
		}
		else if (command == "report-util") {
			GlobalScheduler::getInstance()->logToFile();
		}
		else if (command == "exit") {
			ConsoleManager::getInstance()->exitApplication();
		}
		else if (command == "scheduler-test")
		{
			GlobalScheduler::getInstance()->startSchedulerTest();
		}
		else if (command == "scheduler-stop")
		{
			GlobalScheduler::getInstance()->stopSchedulerTest();
		}

	}

	void getUserInput(std::string& userInput) {
		std::cout << "Enter a command: ";
		std::getline(std::cin >> std::ws, userInput);
	}


	// Function to truncate text with ellipsis at the front if it exceeds the column width
	std::string& truncateText(std::string& text, int maxWidth) {
		if (text.length() > maxWidth) {
			text = "..." + text.substr(text.length() - (maxWidth - 3));  // Truncate and add "..." at the start
		}
		return text;
	}
}


void MainConsole::onEnabled() {
	display();
	process();
}

void MainConsole::process() {
	std::string userInput;

	while (ConsoleManager::getInstance()->isRunning()) {
		MainConsoleUtil::getUserInput(userInput);

		if (!MainConsoleUtil::initialized)
		{
			if (userInput == "initialize")
			{
				MainConsoleUtil::onEvent(userInput);
				MainConsoleUtil::initialized = true;
			}
			else
			{
				std::cout << "Please initialize the program first.\n";
			}
		}
		else
		{
			MainConsoleUtil::onEvent(userInput);
		}


		//for debugging
		std::cout << "Cpu cycles: " << cpuCycles << '\n';
	}
}

void MainConsole::display() {
	std::cout << asciiHeader << '\n';
	std::cout << "\033[32m" << "Hello, Welcome to CSOPESY commandline!\n";
	std::cout << "\033[93m" << "Type \'exit\' to quit, \'clear\' to clear the screen\n";
	std::cout << "\033[0m"; // reset color
}