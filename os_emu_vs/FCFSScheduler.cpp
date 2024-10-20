#include "FCFSScheduler.h"
#include <iostream>

FCFSScheduler::FCFSScheduler()
    : AScheduler(SchedulingAlgorithm::FCFS) {
}

void FCFSScheduler::init() {
}

void FCFSScheduler::execute() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // Simulate waiting or other periodic tasks
    }
}
