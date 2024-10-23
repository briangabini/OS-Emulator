#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <chrono>
#include <vector>

class Command;

class Process {
public:
    Process(const std::string& name);
    ~Process();

    const std::string& getName() const;
    int getId() const;

    void addCommand(Command* cmd);
    Command* getNextCommand();

    void log(const std::string& message, int coreId);

    std::time_t getCreationTime() const;

    int getCurrentLine() const;
    int getTotalLines() const;
    std::string getCurrentCodeLine() const;

    bool isCompleted() const;
    void setCompleted(bool value);
    void resetCompleted();
    void incrementCurrentLine();

private:
    std::string name;
    int id;

    static int nextId;

    std::queue<Command*> commandQueue;
    mutable std::mutex queueMutex;

    std::chrono::system_clock::time_point creationTime;

    std::vector<std::string> codeLines;
    int currentLine;
    int totalLines;
    bool completed;

    mutable std::mutex stateMutex;

    std::mutex logMutex;
};
