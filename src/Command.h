#pragma once

#include <string>

class Process;

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(Process* process, int coreId) = 0;
    virtual std::string getDescription() const = 0;
};
