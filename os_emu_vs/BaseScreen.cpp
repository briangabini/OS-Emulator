#include "BaseScreen.h"
#include "ConsoleManager.h"
#include "Process.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#define _WIN32

// Definition of the static member variable
bool BaseScreen::activeScreen = true;

namespace {
	void onEvent(BaseScreen& screen, const std::string_view command) {

		if (command == "exit") {
			//ConsoleManager::getInstance()->switchConsole(MAIN_CONSOLE);
			ConsoleManager::getInstance()->returnToPreviousConsole();
			BaseScreen::setActiveScreen(false);
		}
		else if (command == "process-smi") {
			screen.displaySMI();
		}
		else {
			std::cout << "Command not recognized. Please try again." << '\n';
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

void BaseScreen::printProcessSMI() const {
	if (attachedProcess->isFinished()) {
		std::cout << "Finished!" << '\n';
	}
	else {
		std::cout << "Process: " << attachedProcess->getName() << '\n';

		std::cout << "ID: " << attachedProcess->getPID() << "\n\n";

		std::cout << "Current instruction line: " << attachedProcess->getCommandCounter() << '\n';

		std::cout << "Lines of code: " << attachedProcess->getLinesOfCode() << '\n';
	}


}

void BaseScreen::onEnabled() {
	display();
	process();
};

void BaseScreen::process() {
	std::string userInput;
	while (activeScreen) {
		getUserInput(userInput);
		onEvent(*this, userInput);
	}
};

void BaseScreen::display() {
	printProcessInfo();
};

void BaseScreen::displaySMI() {
	printProcessSMI();
};

BaseScreen::BaseScreen(std::shared_ptr<Process> process, const String& processName)
	: AConsole(processName), attachedProcess(process), timestamp(getCurrentTimestamp())
{
}

