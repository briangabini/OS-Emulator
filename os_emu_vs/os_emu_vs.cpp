// os_emu_vs.cpp
#include "ConsoleManager.h"
#include "GlobalScheduler.h"

#include "MemoryManager.h"
#include <iostream>

int main() {
	ConsoleManager::initialize();
	bool running = true;

	// start incrementing cpuCycle in another thread
	while (running) {

		ConsoleManager::getInstance()->drawConsole();
		ConsoleManager::getInstance()->process();

		running = ConsoleManager::getInstance()->isRunning();
	}

	// clean up
	ConsoleManager::destroy();
	GlobalScheduler::destroy();
	MemoryManager::destroy();

	std::cout << "Exiting the program..." << '\n';

	return 0;
}