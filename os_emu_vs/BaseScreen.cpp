#include "BaseScreen.h"
#include <string>
#include <memory>
#include <chrono>
#include <iostream>
#include "Process.h"
#include "ConsoleManager.h"

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
    std::cout << *attachedProcess;

	std::cout << "Created at: " << getTimestamp() << '\n';
}

void BaseScreen::onEnabled() {
    display();
};

void BaseScreen::process() {
	ConsoleManager::getInstance()->drawConsole();
};

void BaseScreen::display() {
    printProcessInfo();
};

BaseScreen::BaseScreen(std::shared_ptr<Process> process, const String& processName)
	: AConsole(processName), attachedProcess(process)
{
}

