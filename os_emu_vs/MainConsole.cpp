#include "BaseScreen.h"
#include "ConsoleManager.h"
#include "MainConsole.h"
#include <array>
#include <iostream>
#include <sstream>
#include <vector>

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

	inline constexpr int commandsCount = 8;

	inline constexpr std::array<std::string_view, commandsCount> commands = {
		"initialize",
		"screen",
		"scheduler-test",
		"scheduler-stop",
		"report-util",
		"nvidia-smi",
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

	void gpuSummary() {
		struct GPUInfo {
			std::string id;
			std::string name;
			std::string driverModel;
			std::string busId;
			std::string dispA;
			std::string ecc;
			std::string fan;
			std::string temp;
			std::string perf;
			std::string powerUsage;
			std::string memoryUsage;
			std::string gpuUtil;
			std::string computeM;
			std::string migM;
		};

		struct ProcessInfo {
			std::string pid;
			std::string type;
			std::string processName;
			std::string gpuMemoryUsage;
		};

		std::vector<GPUInfo> gpus = {
			{"0", "NVIDIA GeForce RTX 3060", "WDDM", "00000000:01:00.0",
				"On", "N/A", "0%", "44C",
				"P8", "12W /  170W", "2743MiB /  12288MiB", "2%",
				"Default", "N/A"}
		};

		std::vector<ProcessInfo> processes = {
			{"22104", "C+G", "C:\\Windows\\System32\\ServiceHub.ThreadedWaitDialog.exe", "N/A"},
			{"13848", "C+G", "C:\\Program Files\\Discord\\Discord.exe", "N/A"},
			{"30648", "C+G", "C:\\Windows\\System32\\SystemSettings.exe", "N/A"},
			{"6888", "C+G", "C:\\Windows\\explorer.exe", "N/A"},
			{"7252", "C+G", "C:\\Program Files\\WindowsApps\\Microsoft.Media.Player.exe", "N/A"}
		};

		auto truncateString = [](const std::string& str, std::size_t width) {
			if (str.length() > width) {
				return "..." + str.substr(str.length() - (width - 3));
			}
			return str;
			};

		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm now_tm;
		localtime_s(&now_tm, &now_time);

		std::cout << std::put_time(&now_tm, "%a %b %d %H:%M:%S %Y") << "\n";
		std::cout << "+-----------------------------------------------------------------------------------------+\n";
		std::cout << "| NVIDIA-SMI 561.09                 Driver Version: 561.09         CUDA Version: 12.6     |\n";
		std::cout << "|-----------------------------------------+------------------------+----------------------+\n";
		std::cout << "| GPU  Name                  Driver-Model | Bus-Id          Disp.A | Volatile Uncorr. ECC |\n";
		std::cout << "| Fan  Temp   Perf          Pwr:Usage/Cap |           Memory-Usage | GPU-Util  Compute M. |\n";
		std::cout << "|                                         |                        |               MIG M. |\n";
		std::cout << "|=========================================+========================+======================|\n";

		for (GPUInfo gpu : gpus) {
			std::cout << "| " << std::setw(3) << gpu.id
				<< "  " << std::setw(22) << gpu.name
				<< "   " << std::setw(7) << gpu.driverModel << " "
				<< " |   " << std::setw(16) << gpu.busId
				<< "  " << std::setw(2) << gpu.dispA
				<< " | " << std::setw(20) << gpu.ecc << " |\n";
			std::cout << "|" << std::setw(4) << gpu.fan
				<< "  " << std::setw(4) << gpu.temp
				<< "  " << std::setw(4) << gpu.perf
				<< "  " << std::setw(22) << gpu.powerUsage
				<< " | " << std::setw(22) << gpu.memoryUsage
				<< " | " << std::setw(7) << gpu.gpuUtil
				<< "   " << std::setw(10) << gpu.computeM << " |\n";
			std::cout << "|                                         |                        | " << std::setw(20) << gpu.migM << " |\n";
		}

		std::cout << "+-----------------------------------------+------------------------+----------------------+\n\n";
		std::cout << "+----------------------------------------------------------------------+\n";
		std::cout << "| Processes:                                                           |\n";
		std::cout << "|    PID   Type   Process name                              GPU Memory |\n";
		std::cout << "|                                                           Usage      |\n";
		std::cout << "|======================================================================|\n";

		for (ProcessInfo process : processes) {
			std::cout << "| " << std::setw(6) << process.pid
				<< "   " << std::setw(4) << process.type
				<< "   " << std::left << std::setw(39) << truncateString(process.processName, 38)
				<< " " << std::right << std::setw(7) << process.gpuMemoryUsage << "      |\n";
		}

		std::cout << "+----------------------------------------------------------------------+\n";
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

		if (command == "nvidia-smi")
		{
			gpuSummary();
		}
		else if (command == "clear") {
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