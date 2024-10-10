#include "MainConsole.h"
#include <iostream>
#include <array>
#include <vector>
#include <sstream>
#include "BaseScreen.h"
#include "ConsoleManager.h"
#include "GlobalScheduler.h"
#include "TypedefRepo.h"

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

namespace {
	// forward declarations
	//void printNvidiaSmiOutput();

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

				//const auto newProcess = std::make_shared<Process>(processName);
				//const auto newBaseScreen = std::make_shared<BaseScreen>(newProcess, processName);

				try {
					//ConsoleManager::getInstance()->registerScreen(newBaseScreen);
					//ConsoleManager::getInstance()->switchToScreen(processName);
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
			} else if (flag == "-ls")
			{
				//GlobalScheduler::getInstance()->monitorProcesses();
				GlobalScheduler::getInstance()->listProcesses();
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
		else if (command == "nvidia-smi") {
			//printNvidiaSmiOutput();
		} else if (command == "marquee")
		{
			ConsoleManager::getInstance()->switchConsole(MARQUEE_CONSOLE);
		}
		else if (command == "exit") {
			ConsoleManager::getInstance()->exitApplication();
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

	//void printNvidiaSmiOutput() {
	//	constexpr std::string_view gpuSummary = "Mon Sep 30 00:19:57 2024\n"
	//		"+-----------------------------------------------------------------------------------------+\n"
	//		"| NVIDIA-SMI 561.09                 Driver Version: 561.09         CUDA Version: 12.6     |\n"
	//		"|-----------------------------------------+------------------------+----------------------+\n"
	//		"| GPU  Name                  Driver-Model | Bus-Id          Disp.A | Volatile Uncorr. ECC |\n"
	//		"| Fan  Temp   Perf          Pwr:Usage/Cap |           Memory-Usage | GPU-Util  Compute M. |\n"
	//		"|                                         |                        |               MIG M. |\n"
	//		"|=========================================+========================+======================|\n"
	//		"|   0  NVIDIA GeForce RTX 2060 ...  WDDM  |   00000000:26:00.0  On |                  N/A |\n"
	//		"| 41%   43C    P8             25W /  184W |    2684MiB /   8192MiB |     14%      Default |\n"
	//		"|                                         |                        |                  N/A |\n"
	//		"+-----------------------------------------+------------------------+----------------------+\n";

	//	constexpr std::string_view gpuProcessHeader =
	//		"+-------------------------------------------------------------------------+\n"
	//		"| Processes:                                                              |\n"
	//		"|      PID   Type    Process name                              GPU Memory |\n"
	//		"|                                                              Usage      |\n"
	//		"|=========================================================================|\n";

	//	constexpr std::string_view gpuProcessFooter =
	//		"+-------------------------------------------------------------------------+\n";

	//	// Sample data for the rows
	//	std::vector<std::vector<std::string>> processTable = {
	//		{"2268", "C+G", "C:\\Users\\User\\AppData\\Local\\Discord\\app-1.0.9164\\Discord.exe", "N/A"},
	//		{"4168", "C+G", "C:\\Program Files\\qemu-windows-x86_64\\qemu-system-x86_64.exe", "N/A"},
	//		{"4560", "C+G", "C:\\Windows\\SystemApps\\Microsoft.Windows.ShellExperienceHost_cw5n1h2txyewy\\ShellExperienceHost.exe", "N/A"},
	//		{"511900", "C+G", "C:\\Windows\\explorer.exe", "N/A"},
	//		{"81120", "C+G", "C:\\Windows\\SystemApps\\MicrosoftWindows.Client.CBS_cw5n1h2txyewy\\SearchHost.exe", "N/A"}
	//	};


	//	std::cout << gpuSummary ;
	//	std::cout << '\n';
	//	std::cout << gpuProcessHeader;

	//	for (auto& row : processTable) {
	//		// Define column widths
	//		constexpr int pidWidth = 6;        // Width for the PID column
	//		constexpr int typeWidth = 3;       // Width for the Type column
	//		constexpr int nameWidth = 38;      // Width for the Process Name column
	//		constexpr int memoryWidth = 10;    // Width for the GPU Memory Usage column

	//		// Get each column's text, truncate if necessary (modified directly)
	//		std::string& pid = truncateText(row[0], pidWidth);
	//		std::string& type = truncateText(row[1], typeWidth);
	//		std::string& processName = truncateText(row[2], nameWidth);
	//		std::string& memoryUsage = truncateText(row[3], memoryWidth);

	//		// Print the row with proper column spacing
	//		std::cout << "|   " << std::right << std::setw(pidWidth) << pid
	//			<< "    " << std::right << std::setw(typeWidth) << type
	//			<< "    " << std::left << std::setw(nameWidth) << processName
	//			<< "    " << std::left << std::setw(memoryWidth) << memoryUsage << " |\n";
	//	}

	//	std::cout << gpuProcessFooter;
	//}
}


void MainConsole::onEnabled() {
	display();
	process();
}

void MainConsole::process() {
	std::string userInput;
	while (ConsoleManager::getInstance()->isRunning()) {
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