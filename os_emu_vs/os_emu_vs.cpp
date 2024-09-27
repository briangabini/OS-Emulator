// os_emu_vs.cpp
#include "ConsoleManager.h"

int main() {
	ConsoleManager::initialize();

	bool running = true;
	while (running) {
		ConsoleManager::getInstance()->process();
		ConsoleManager::getInstance()->drawConsole();

		running = ConsoleManager::getInstance()->isRunning();
	}

	// clean up
	ConsoleManager::destroy();

	return 0;
}