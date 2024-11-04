#pragma once
#include "AScheduler.h"

class RRScheduler : public AScheduler {
public:
    RRScheduler ();
    void init() override;
    void execute() override;
};
#pragma once
