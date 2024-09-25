#pragma once
#include <array>
#include <cstdlib>
#include <iostream>
#include <string_view>

#define windows // change based on your OS

namespace Util {
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

	inline void flushScreen() {
		std::cout << std::flush;
	}

	inline void greetings() {
		std::cout << Util::asciiHeader << '\n';
		std::cout << "\033[32m" << "Hello, Welcome to CSOPESY commandline!\n";
		std::cout << "\033[93m" << "Type \'exit\' to quit, \'clear\' to clear the screen\n";
		std::cout << "\033[0m"; // reset color
	}


	inline void clearScreen() {
		flushScreen();

	#ifdef windows
		std::system("cls");
	#else
		std::system("clear");
	#endif
	}

	
}
