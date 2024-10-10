#include "Command.h"
#include "Process.h"
#include <thread>
#include <chrono>

PrintCommand::PrintCommand(const std::string& message)
    : message(message) {}

void PrintCommand::execute(Process* process, int coreId) {
    // Simulate execution time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Log the message
    process->log(message, coreId);
}

std::string PrintCommand::getDescription() const {
    return "print " + message;
}
