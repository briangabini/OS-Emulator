#pragma once

#include <string>
#include <ctime>
#include <vector>
#include <mutex>
#include <chrono>

class Process {
public:
    Process(const std::string& name);
    ~Process();

    void run();
    std::string getName() const;
    std::time_t getCreationTime() const;
    int getCurrentLine() const;
    int getTotalLines() const;
    std::string getCurrentCodeLine() const;
    bool isCompleted() const;

private:
    std::string name;
    std::vector<std::string> codeLines;
    int currentLine;
    int totalLines;
    bool isRunning;
    std::chrono::system_clock::time_point creationTime;
    std::thread processThread;
    mutable std::mutex mutex;
};