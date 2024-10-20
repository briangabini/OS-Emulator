#include "PrintCommand.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

PrintCommand::PrintCommand(int pid, String& toPrint) : ICommand(pid, PRINT) {
	this->toPrint = toPrint;
}

void PrintCommand::execute() {
    ICommand::execute();
}
