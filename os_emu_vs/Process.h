#pragma once
#include <string>
#include <vector>
#include <chrono>

class Process {
public:
    explicit Process(const std::string& name);
    Process() = default;

    std::string getName() const;
    int getCurrentLine() const;
    int getTotalLines() const;

    friend std::ostream& operator<<(std::ostream& out, const Process& process);

private:
    std::string name;
    int currentLine;
    int totalLines;
};