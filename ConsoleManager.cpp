#include "ConsoleManager.h"
#include "MainConsole.h"
#include "Marquee.h"
#include <iostream>

ConsoleManager::ConsoleManager() {
    mainConsole = new MainConsole(*this);
}

ConsoleManager::~ConsoleManager() {
    delete mainConsole;
    for (auto& pair : processes) {
        delete pair.second;
    }
}

void ConsoleManager::start() {
    mainConsole->run();
}

void ConsoleManager::switchToMainConsole() {
    mainConsole->run();
}

void ConsoleManager::switchToScreen(Process* process) {
    Screen screen(*this, process);
    screen.run();
}

void ConsoleManager::switchToMarquee() {
    Marquee marquee(*this);
    marquee.run();
}

std::mutex& ConsoleManager::getIOMutex() {
    return ioMutex;
}

void ConsoleManager::createProcess(const std::string& name) {
    std::lock_guard<std::mutex> lock(processMutex);
    if (processes.find(name) == processes.end()) {
        processes[name] = new Process(name);
    }
    else {
        std::cout << "Process with name '" << name << "' already exists.\n";
    }
}

Process* ConsoleManager::getProcess(const std::string& name) {
    std::lock_guard<std::mutex> lock(processMutex);
    auto it = processes.find(name);
    if (it != processes.end()) {
        return it->second;
    }
    else {
        std::cout << "No process found with name '" << name << "'.\n";
        return nullptr;
    }
}
