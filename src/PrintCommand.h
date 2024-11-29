#pragma once

#include "Command.h"

class PrintCommand : public Command {
public:
    PrintCommand(const std::string& message);
    void execute(Process* process, int coreId) override;
    std::string getDescription() const override;

private:
    std::string message;
};
