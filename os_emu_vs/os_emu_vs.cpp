// os_emu_vs.cpp
#include "ConsoleManager.h"
#include "GlobalScheduler.h"

#include "FlatMemoryAllocator.h"
#include <iostream>

int cpuCycles = 0;

int main() {
	ConsoleManager::initialize();
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

	cpuThread.join();

	// clean up
	ConsoleManager::destroy();
	GlobalScheduler::destroy();
	FlatMemoryAllocator::destroy();

	std::cout << "Exiting the program..." << '\n';

	return 0;
}