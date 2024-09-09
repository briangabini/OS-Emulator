#ifndef UTIL_H
#define UTIL_H

#include <array>
#include <cstdlib>
#include <iostream>

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

	inline constexpr std::array<const char*, commandsCount> commands = {
		"initialize",
		"screen",
		"scheduler-test",
		"scheduler-stop",
		"report-util",
		"clear",
		"exit"
	};

	inline bool checkIfCommandExists(const std::string_view command) {
		return std::ranges::find_if(commands, [&command](const char* cmd) {
			return command == cmd;
			}) != commands.end();
	}

	inline void flushScreen() {
		std::cout << std::flush;
	}

	inline void greetings() {
		std::cout << Util::asciiHeader << '\n';
		std::cout << "Hello, Welcome to CSOPESY commandline!\n";
		std::cout << "Type \'exit\' to quit, \'clear\' to clear the screen\n";
	}


	inline void clearScreen() {
		flushScreen();

	#ifdef windows
		std::system("cls");
	#else
		std::system("clear");
	#endif

		greetings();
	}

	
}

#endif //UTIL_H