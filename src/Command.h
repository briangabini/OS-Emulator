#pragma once

#include <string>

class Process;

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(Process* process, int coreId) = 0;
    virtual std::string getDescription() const = 0;
};

class PrintCommand : public Command {
public:
    PrintCommand(const std::string& message);
    void execute(Process* process, int coreId) override;
    std::string getDescription() const override;

private:
    std::string message;
};
