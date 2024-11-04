#pragma once
#include <string>
#include <string_view>

extern int cpuCycles;

void getUserInput(std::string& userInput);

void onEvent(std::string_view command);
