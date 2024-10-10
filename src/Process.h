#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <chrono>
#include <ctime>
#include <vector>
#include <mutex>

class Command;

class Process {
public:
    Process(const std::string& name);
    ~Process();

    const std::string& getName() const;
    void addCommand(Command* cmd);
    Command* getNextCommand();

    void log(const std::string& message, int coreId);

    // Methods for process information
    std::time_t getCreationTime() const;
    int getCurrentLine() const;
    int getTotalLines() const;
    std::string getCurrentCodeLine() const;
    bool isCompleted() const;

    void incrementCurrentLine();
    void setCompleted(bool value);

    // For Scheduler access
    mutable std::mutex stateMutex;

private:
    std::string name;
    std::queue<Command*> commandQueue;
    std::mutex queueMutex;

    // For logging
    std::mutex logMutex;

    // Process state
    std::chrono::system_clock::time_point creationTime;
    int currentLine;
    int totalLines;
    bool completed;

    std::vector<std::string> codeLines;
};
