#pragma once
#include "AScheduler.h"
#include "MemoryManager.h"

class RRScheduler : public AScheduler {
public:
    RRScheduler();
    void init() override;
    void execute() override;

private:
    size_t quantumCount = 0;
};