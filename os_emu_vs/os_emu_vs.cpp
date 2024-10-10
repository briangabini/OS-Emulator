// os_emu_vs.cpp
#include "ConsoleManager.h"
#include "GlobalScheduler.h"

int main() {
	ConsoleManager::initialize();
	GlobalScheduler::initialize();

	GlobalScheduler::getInstance()->test_init100Processes();

	// switch to main console
	ConsoleManager::getInstance()->switchConsole(MAIN_CONSOLE);

	/*bool running = true;
	while (running) {
		ConsoleManager::getInstance()->process();
		ConsoleManager::getInstance()->drawConsole();

		running = ConsoleManager::getInstance()->isRunning();
	}*/

	// clean up
	ConsoleManager::destroy();
	GlobalScheduler::destroy();

	return 0;
}