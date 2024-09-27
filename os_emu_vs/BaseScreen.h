#pragma once
#include <memory>
#include <string>
#include "AConsole.h"
#include "Process.h"

class BaseScreen : public AConsole
{
public:
    using String = std::string;

    // Constructor that requires a Process and processName
    BaseScreen(std::shared_ptr<Process> process, const String& processName);

    // Default constructor (if needed)
    BaseScreen() : attachedProcess(nullptr), refreshed(false), timestamp(getCurrentTimestamp()) {}

    ~BaseScreen() override = default;
    void onEnabled() override;
    void process() override;
    void display() override;

    std::string getTimestamp() const;

private:
    void printProcessInfo() const;
    std::shared_ptr<Process> attachedProcess;
    bool refreshed = false;
    std::string timestamp;

    std::string getCurrentTimestamp() const;
};