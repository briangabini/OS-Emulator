#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <string_view>

void getUserInput(std::string& userInput);

void onEvent(std::string_view command);

#endif //MAIN_H