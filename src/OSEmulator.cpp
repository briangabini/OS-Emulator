#include "ConsoleManager.h"
#include <iostream>

int main() {
    std::cout << R"(  _____   _____    _____   _____    _______   _____   __    __ )" << "\n";
    std::cout << R"( / ____| / ____|  /  __ \  |  __ \  |  ____| / ____|  \ \  / / )" << "\n";
    std::cout << R"(| |      | (___   | |  | | | |__) | | |__    | (___    \ \/ /  )" << "\n";
    std::cout << R"(| |       \___ \  | |  | | |  ___/  |  __|    \___ \    |  |   )" << "\n";
    std::cout << R"(| |____   ____) | | |__| | | |      | |____   ____) |   |  |   )" << "\n";
    std::cout << R"( \_____| |_____/  \_____/  |_|      |______| |_____/    |__|   )" << "\n";
    std::cout << "--------------------------------------------------------------\n\n";
    std::cout << "Welcome to CSOPESY Emulator!\n\n";
    std::cout << "Developers:\n";
    std::cout << "Gabini, Brian\n";
    std::cout << "Recato Dy, John Kieffer\n";
    std::cout << "Tan, Timothy Joshua\n";
    std::cout << "Verano, Carl Matthew\n\n";
    std::cout << "Last updated: 11-25-2024\n\n";

    ConsoleManager consoleManager;
    consoleManager.start();
    return 0;
}
