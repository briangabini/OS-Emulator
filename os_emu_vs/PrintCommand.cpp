#include "PrintCommand.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

PrintCommand::PrintCommand(int pid, String& toPrint) : ICommand(pid, PRINT) {
    this->toPrint = toPrint;
}

void PrintCommand::execute() {
    ICommand::execute();
}
