#pragma once
#include "ICommand.h"
#include "TypedefRepo.h"

class PrintCommand : public ICommand {
public:
	PrintCommand(int pid, String& toPrint);
	void execute(int cpuCoreId) override;

private:
	String toPrint;
	void logToFile(const String& message);
};
