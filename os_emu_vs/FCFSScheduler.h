#pragma once
#include "AScheduler.h"

class FCFSScheduler : public AScheduler {
public:
    FCFSScheduler();
    void init() override;
    void execute() override;
};
