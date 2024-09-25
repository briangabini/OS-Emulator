// Screen.h
#pragma once
#include "Process.h"
#include <string>
#include <map>

class Screen {
public:
    void createScreen(const std::string& processName);
    void reattachScreen(const std::string& processName);
    void handleScreenCommand(const std::string& processName);

private:
    std::map<std::string, Process> screens;
};