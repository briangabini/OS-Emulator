#include "RRScheduler.h"
#include "GlobalConfig.h"
#include "os_emu_vs.h" 
#include <iostream>

RRScheduler::RRScheduler()
    : AScheduler(SchedulingAlgorithm::ROUND_ROBIN) {
}

void RRScheduler::init() {
    quantumCount = 0;
}

void RRScheduler::execute() {
    size_t quantumCycles = GlobalConfig::getInstance()->getQuantumCycles();
    size_t lastSnapshotTime = 0;

    while (running) {
        // Generate memory snapshot at quantum boundaries
        if (cpuCycles >= (lastSnapshotTime + quantumCycles)) {
            try {
                MemoryManager::getInstance()->generateMemorySnapshot(quantumCount++);
                lastSnapshotTime = cpuCycles;
            }
            catch (const std::exception& e) {
                // Log error if needed
            }
        }

        // Let the workers handle all process management
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}