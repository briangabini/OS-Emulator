// os_emu_vs.cpp
#include "ConsoleManager.h"
#include "GlobalScheduler.h"

#include <iostream>

int cpuCycles = 0;

int main() {
	ConsoleManager::initialize();
	GlobalScheduler::initialize();

	// GlobalScheduler::getInstance()->test_init100Processes();

	//std::this_thread::sleep_for(std::chrono::seconds(5));

	bool running = true;

	// start incrementing cpuCycle in another thread
	std::thread cpuThread([&running]() {
		while (running) {
			cpuCycles++;
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 0.1 second
		}
		});

	while (running) {

		ConsoleManager::getInstance()->drawConsole();
		ConsoleManager::getInstance()->process();

		running = ConsoleManager::getInstance()->isRunning();
	}

	 // switch to main console
	 //ConsoleManager::getInstance()->switchConsole(MAIN_CONSOLE);


	cpuThread.join();

	// clean up
	ConsoleManager::destroy();
	GlobalScheduler::destroy();

	std::cout << "Exiting the program..." << '\n';

	return 0;
}