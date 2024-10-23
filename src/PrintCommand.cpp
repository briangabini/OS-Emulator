#include "PrintCommand.h"
#include "Process.h"
#include <thread>
#include <chrono>
#include "Config.h"

PrintCommand::PrintCommand(const std::string& message)
    : message(message) {}

void PrintCommand::execute(Process* process, int coreId) {
    unsigned int delay = Config::getInstance().getDelaysPerExec();
    if (delay > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    process->log(message, coreId);
}

std::string PrintCommand::getDescription() const {
    return "print " + message;
}
