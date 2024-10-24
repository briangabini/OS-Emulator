#pragma once
#include "AConsole.h"
#include "Process.h"
#include <memory>
#include <string>

class BaseScreen : public AConsole
{
public:
	using String = std::string;

	// Constructor that requires a Process and processName
	BaseScreen(std::shared_ptr<Process> process, const String& processName);

	// Default constructor (if needed)
	BaseScreen() : attachedProcess(nullptr), timestamp(getCurrentTimestamp()) {}

	~BaseScreen() override = default;
	void onEnabled() override;
	void process() override;
	void display() override;
	void displaySMI();
	void printProcessSMI() const;

	std::string getTimestamp() const;

	static void setActiveScreen(bool boolVal) {
		activeScreen = boolVal;
	}

private:
	void printProcessInfo() const;
	std::shared_ptr<Process> attachedProcess;
	bool refreshed = false;
	std::string timestamp;

	std::string getCurrentTimestamp() const;

	static bool activeScreen;

};