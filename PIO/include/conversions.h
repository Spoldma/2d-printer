#include "config.h"

struct StepIntervals {
    uint32_t x;
    uint32_t y;
};

struct StepCount {
    uint32_t x;
    uint32_t y;
};

StepIntervals calculateStepIntervals(StepCount steps);

StepCount calculateStepCount(uint32_t dx, uint32_t dy);