#include "os_emu_vs.h"
#include "util.h"
#include <iostream>
#include <string>
#include <string_view>

int main() {
	std::string userInput{};

	Util::greetings();

	while (true) {
		getUserInput(userInput);

		onEvent(userInput);
	}

	return 0;
}


void getUserInput(std::string& userInput) {
	std::cout << "Enter a command: ";
	std::getline(std::cin >> std::ws, userInput);
}

void onEvent(const std::string_view command) {
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