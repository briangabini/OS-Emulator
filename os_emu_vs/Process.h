// Process.h
#pragma once
#include <string>
#include <vector>
#include <chrono>

class Process {
public:
    explicit Process(const std::string& name);
    Process();

    std::string getName() const;
    int getCurrentLine() const;
    int getTotalLines() const;
    std::string getTimestamp() const;

private:
    std::string name;
    std::string timestamp;
    int currentLine;
    int totalLines;

    std::string getCurrentTimestamp() const;
};