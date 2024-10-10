#include "ConsoleManager.h"

#include <iostream>

#include "MainConsole.h"
#include "MarqueeConsole.h"
// #include "SchedulingConsole.h"
// #include "MemorySimulationConsole.h"

// initialize the shared instance
ConsoleManager* ConsoleManager::sharedInstance = nullptr;

ConsoleManager* ConsoleManager::getInstance()
{
	return sharedInstance;
}

void ConsoleManager::initialize() {
	if (sharedInstance == nullptr) {
		sharedInstance = new ConsoleManager();
	}
}

void ConsoleManager::destroy()
{
	delete sharedInstance;
	sharedInstance = nullptr;
}

void ConsoleManager::drawConsole() const
{
	if (this->currentConsole != nullptr)
	{
		this->currentConsole->display();
	}
	else
	{
		std::cerr << "There is no assigned console. Please check." << '\n';
	}
}

void ConsoleManager::process() const
{
	if (this->currentConsole != nullptr)
	{
		this->currentConsole->process();
	}
	else
	{
		std::cerr << "There is no assigned console. Please check." << '\n';
	}
}

void ConsoleManager::switchConsole(const String& consoleName)
{
	if (this->consoleTable.contains(consoleName))
	{
		// Clear the screen
		system("cls");
		this->previousConsole = this->currentConsole;
		this->currentConsole = this->consoleTable[consoleName];
		this->currentConsole->onEnabled();
	}
	else
	{
		std::cerr << "Console name " << consoleName << " not found. Was it initialized?" << '\n';
	}
}

void ConsoleManager::registerScreen(std::shared_ptr<BaseScreen> screenRef)
{
    if (this->consoleTable.contains(screenRef->getName()))
    {
        std::cerr << "Screen name " << screenRef->getName() << " already exists. Please use a different name." << '\n';
		throw std::runtime_error("Screen name already exists.");
    }

    this->consoleTable[screenRef->getName()] = screenRef;
}

void ConsoleManager::switchToScreen(const String& screenName)
{
	if (this->consoleTable.contains(screenName))
	{
		// Clear the screen
		system("cls");
		this->previousConsole = this->currentConsole;
		this->currentConsole = this->consoleTable[screenName];
		this->currentConsole->onEnabled();
	}
	else
	{
		std::cerr << "Screen name " << screenName << " not found. Was it initialized?" << '\n';
	}
}

void ConsoleManager::unregisterScreen(const String& screenName)
{
	if (this->consoleTable.contains(screenName))
	{
		this->consoleTable.erase(screenName);
	}
	else
	{
		std::cerr << "Unable to unregister " << screenName << ". Was it registered?" << '\n';
	}
}

void ConsoleManager::returnToPreviousConsole()
{
	if (this->previousConsole != nullptr)
	{
		// Clear the screen
		system("cls");
		this->currentConsole = this->previousConsole;
		this->previousConsole = nullptr;
	}
	else
	{
		std::cerr << "There is no previous console to return to." << '\n';
	}
}

void ConsoleManager::exitApplication() {
	this->running = false;
}

ConsoleManager::ConsoleManager()
{
	this->running = true;

	// initialize consoles
	// this->consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	const auto mainConsole    = std::make_shared<MainConsole>();
	const auto marqueeConsole = std::make_shared<MarqueeConsole>();
	// const std::shared_ptr<SchedulingConsole> schedulingConsole = std::make_shared<SchedulingConsole>();
	// const std::shared_ptr<MemorySimulationConsole> memoryConsole = std::make_shared<MemorySimulationConsole>();

	this->consoleTable[MAIN_CONSOLE] = mainConsole;
	this->consoleTable[MARQUEE_CONSOLE] = marqueeConsole;
	// this->consoleTable[SCHEDULING_CONSOLE] = schedulingConsole;
	// this->consoleTable[MEMORY_CONSOLE] = memoryConsole;
}

bool ConsoleManager::isRunning() const
{
	return this->running;
}
