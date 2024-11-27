#include "MainConsole.h"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "FlatMemoryAllocator.h"
#include "os_emu_vs.h"
#include "BaseScreen.h"
#include "ConsoleManager.h"
#include "GlobalConfig.h"
#include "GlobalScheduler.h"
#include "MemoryManager.h"
#include "TypedefRepo.h"
#include "Process.h"


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

	inline constexpr int commandsCount = 10;

	inline constexpr std::array<std::string_view, commandsCount> commands = {
		"initialize",
		"screen",
		"scheduler-test",
		"scheduler-stop",
		"report-util",
		"clear",
		"exit",
		"process-smi",
		"vmstat",
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
	void displayVmstat();
	void onEvent(const std::string_view command);
	void getUserInput(std::string& userInput);
	void processSmi();


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
			// HOTFIX: sleep for 2 seconds
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
		else if (command == "clear") {
			clearScreen();
		}
		else if (command == "process-smi") {
			processSmi();
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
		else if (command == "vmstat") {
			displayVmstat();
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

	void displayVmstat() {
		auto memoryManager = MemoryManager::getInstance();
		int totalMemory = memoryManager->getTotalMemory();
		int usedMemory = memoryManager->getUsedMemory();
		int freeMemory = memoryManager->getFreeMemory();
		int numPagedIn = memoryManager->getNumPagedIn();
		int numPagedOut = memoryManager->getNumPagedOut();

		// Assuming you have functions to get CPU ticks
		int totalCpuTicks = GlobalScheduler::getInstance()->getCpuCycles();
		int activeCpuTicks = GlobalScheduler::getInstance()->getActiveCpuCycles();
		int idleCpuTicks = totalCpuTicks - activeCpuTicks;

		std::cout << "-------------------------------\n";
		std::cout << std::format("{} K total memory\n", totalMemory);
		std::cout << std::format("{} K used memory\n", usedMemory);
		std::cout << std::format("{} K free memory\n", freeMemory);
		std::cout << std::format("{} idle cpu ticks\n", idleCpuTicks);
		std::cout << std::format("{} active cpu ticks\n", activeCpuTicks);
		std::cout << std::format("{} total cpu ticks\n", totalCpuTicks);
		std::cout << std::format("{} num paged in\n", numPagedIn);
		std::cout << std::format("{} num paged out\n", numPagedOut);
		std::cout << "-------------------------------\n";
	}

	void processSmi() {
		double cpuUtil = GlobalScheduler::getInstance()->getCpuUtilization();
		int memoryUsed = MemoryManager::getInstance()->getUsedMemory();
		int totalMemory = MemoryManager::getInstance()->getTotalMemory();
		double memoryUtil = MemoryManager::getInstance()->getMemoryUtilization();

		auto processMapPtr = GlobalScheduler::getInstance()->getProcesses();

		std::cout << "---------------------------------------------\n";
		std::cout << "| PROCESS-SMI V01.00 Driver Version: 01.00 |\n";
		std::cout << "---------------------------------------------\n";
		std::cout << std::format("CPU-Util: {}%\n", cpuUtil);
		std::cout << std::format("Memory Usage: {}KB / {}KB\n", memoryUsed, totalMemory);
		std::cout << std::format("Memory Util: {}%\n", memoryUtil);
		std::cout << "\n=============================================\n";
		std::cout << "Running processes and memory usage:\n";
		std::cout << "---------------------------------------------\n";

		for (const auto& [key, value] : *processMapPtr) {
			if (value->getState() == Process::RUNNING) {
				std::cout << std::format("{}", key);
				std::cout << std::format("\t\t{}KB\n", value->getMemoryRequired());
			}
		}

		std::cout << "---------------------------------------------\n";
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
		//std::cout << "Cpu cycles: " << GlobalScheduler::getInstance()->getCpuCycles() << '\n';
	}
}

void MainConsole::display() {
	std::cout << asciiHeader << '\n';
	std::cout << "\033[32m" << "Hello, Welcome to CSOPESY commandline!\n";
	std::cout << "\033[93m" << "Type \'exit\' to quit, \'clear\' to clear the screen\n";
	std::cout << "\033[0m"; // reset color
}