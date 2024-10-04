#include "MainConsole.h"
#include <iostream>
#include <array>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <random>
#include <algorithm>
#include <conio.h>
#include "BaseScreen.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "MarqueeConsole.h"

// Add Windows-specific include
#include <windows.h>

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
        "nvidia-smi",
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

    std::string truncate(const std::string& str, size_t width, bool show_ellipsis = true) {
        if (str.length() > width) {
            if (show_ellipsis) {
                return str.substr(0, width - 3) + "...";
            }
            else {
                return str.substr(0, width);
            }
        }
        return str;
    }

    void showNvidiaSmi() {
        auto now = std::time(nullptr);
        std::tm localTime;
        localtime_s(&localTime, &now);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> tempDist(30, 80);
        std::uniform_int_distribution<> utilDist(0, 100);
        std::uniform_int_distribution<> memDist(0, 8192);
        std::uniform_int_distribution<> pid4Dist(1000, 9999);
        std::uniform_int_distribution<> pid5Dist(10000, 99999);

        std::cout << std::put_time(&localTime, "%a %b %d %H:%M:%S %Y") << std::endl;
        std::cout << "+-----------------------------------------------------------------------------+\n";
        std::cout << "| NVIDIA-SMI 515.65.01    Driver Version: 515.65.01    CUDA Version: 11.7     |\n";
        std::cout << "|-------------------------------+----------------------+----------------------+\n";
        std::cout << "| GPU  Name        Persistence-M| Bus-Id        Disp.A | Volatile Uncorr. ECC |\n";
        std::cout << "| Fan  Temp  Perf  Pwr:Usage/Cap|         Memory-Usage | GPU-Util  Compute M. |\n";
        std::cout << "|                               |                      |               MIG M. |\n";
        std::cout << "|===============================+======================+======================|\n";

        int temp = tempDist(gen);
        int util = utilDist(gen);
        int memUsed = memDist(gen);
        std::cout << "|   0  GeForce GTX 1080    Off  | 00000000:01:00.0  On |                  N/A |\n";
        std::cout << "| 30%   " << std::setw(2) << temp << "C    P2    88W / 180W |   "
            << std::setw(4) << memUsed << "MiB /  8192MiB |     " << std::setw(2) << util
            << "%      Default |\n";
        std::cout << "|                               |                      |                  N/A |\n";
        std::cout << "+-------------------------------+----------------------+----------------------+\n";

        std::cout << "                                                                               \n";
        std::cout << "+-----------------------------------------------------------------------------+\n";
        std::cout << "| Processes:                                                                  |\n";
        std::cout << "|  GPU   GI   CI        PID   Type   Process name                  GPU Memory |\n";
        std::cout << "|        ID   ID                                                   Usage      |\n";
        std::cout << "|=============================================================================|\n";

        std::vector<std::tuple<int, std::string, int>> processes = {
            {pid4Dist(gen), "C:\\Windows\\System32\\dwm.exe", memDist(gen)},
            {pid4Dist(gen), "C:\\Program Files\\NVIDIA Corporation\\NVIDIA GeForce Experience\\NVIDIA Share.exe", memDist(gen)},
            {pid4Dist(gen), "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe", memDist(gen)},
            {pid5Dist(gen), "C:\\Program Files\\Adobe\\Adobe Photoshop 2023\\Photoshop.exe", memDist(gen)},
            {pid5Dist(gen), "C:\\Program Files\\Epic Games\\Fortnite\\FortniteGame\\Binaries\\Win64\\FortniteClient-Win64-Shipping.exe", memDist(gen)}
        };

        for (const auto& [pid, name, memory] : processes) {
            std::cout << "|    0   N/A  N/A  " << std::setw(8) << pid << "      C   "
                << std::left << std::setw(26) << truncate(name, 26)
                << std::right << std::setw(10) << memory << " MiB |\n";
        }

        std::cout << "+-----------------------------------------------------------------------------+\n";
    }

    void onEvent(const std::string_view command) {
        std::vector<std::string> tokens = splitCommand(std::string(command));

        if (tokens.empty()) {
            std::cout << "Invalid command.\n";
            return;
        }

        else if (tokens[0] == "marquee") {
            ConsoleManager* manager = ConsoleManager::getInstance();
            MarqueeConsole marquee(*manager);
            marquee.run();
            // After marquee exits, redisplay the main console
            system("cls");
            manager->drawConsole();
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
        else if (command == "nvidia-smi") {
            showNvidiaSmi();
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
    std::cout << "\033[93m" << "Type \'exit\' to quit, \'clear\' to clear the screen, or \'nvidia-smi\' to see GPU info\n";
    std::cout << "\033[0m"; // reset color
}