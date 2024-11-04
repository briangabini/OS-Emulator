#include "RRScheduler.h"
#include <iostream>

RRScheduler::RRScheduler()
    : AScheduler(SchedulingAlgorithm::ROUND_ROBIN) {
}

void RRScheduler::init() {
}

void RRScheduler::execute() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // Simulate waiting or other periodic tasks
    }
}
