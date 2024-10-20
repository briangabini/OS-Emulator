#include "BaseScreen.h"
#include <string>
#include <memory>
#include <chrono>
#include <iostream>
#include "Process.h"
#include "ConsoleManager.h"

#define _WIN32

// Definition of the static member variable
bool BaseScreen::activeScreen = true;

namespace {
	void onEvent(const std::string_view command) {

		if (command == "exit") {
			 //ConsoleManager::getInstance()->switchConsole(MAIN_CONSOLE);
			ConsoleManager::getInstance()->returnToPreviousConsole();
			BaseScreen::setActiveScreen(false);
		}
	}

	void getUserInput(std::string& userInput) {
		std::cout << "root:\\>";
		std::getline(std::cin >> std::ws, userInput);
	}
}

std::string BaseScreen::getTimestamp() const {
	return timestamp;
}

std::string BaseScreen::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;

#ifdef _WIN32
    localtime_s(&local_tm, &now_time);
#else
    localtime_r(&now_time, &local_tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%m/%d/%Y, %I:%M:%S %p");
    return oss.str();
}

void BaseScreen::printProcessInfo() const {
	std::cout << "Process Name: " << attachedProcess->getName() << '\n';

	std::cout << "Created at: " << getTimestamp() << '\n';
}

void BaseScreen::onEnabled() {
    display();
	process();
};

void BaseScreen::process() {
	std::string userInput;
	while (activeScreen) {
		getUserInput(userInput);
		onEvent(userInput);
	}
};

void BaseScreen::display() {
    printProcessInfo();
};

BaseScreen::BaseScreen(std::shared_ptr<Process> process, const String& processName)
	: AConsole(processName), attachedProcess(process), timestamp(getCurrentTimestamp())
{
}

