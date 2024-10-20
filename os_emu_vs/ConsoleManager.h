#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <Windows.h>
#include "AConsole.h" // This is an abstract class for the screen
#include "TypedefRepo.h"
#include "BaseScreen.h"

const String MAIN_CONSOLE = "MAIN_CONSOLE";
const String MARQUEE_CONSOLE = "MARQUEE_CONSOLE";
// const String SCHEDULING_CONSOLE = "SCHEDULING_CONSOLE";
// const String MEMORY_CONSOLE = "MEMORY_CONSOLE";

class ConsoleManager
{
public:
	using ConsoleTable = std::unordered_map<String, std::shared_ptr<AConsole>>;

	// for singleton pattern
	static ConsoleManager* getInstance();
	static void initialize();
	static void destroy();

	void drawConsole() const;
	void process() const;
	void switchConsole(const String& consoleName);

	void registerScreen(std::shared_ptr<BaseScreen> screenRef);
	void switchToScreen(const String& screenName);
	void unregisterScreen(const String& screenName);

	void returnToPreviousConsole();
	void exitApplication();
	bool isRunning() const;

	// HANDLE getConsoleHandle() const;

	// void setCursorPosition(int posX, int posY) const;

private:
	ConsoleManager();
	~ConsoleManager() = default;
	ConsoleManager(ConsoleManager const&) {}; // copy constructor is private
	ConsoleManager& operator=(ConsoleManager const&) {}; // assignment operator is private
	static ConsoleManager* sharedInstance;

	ConsoleTable consoleTable;
	std::shared_ptr<AConsole> currentConsole;
	std::shared_ptr<AConsole> previousConsole;

	// HANDLE consoleHandle;
	bool running = true;

};
