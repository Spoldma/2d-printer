#include <algorithm>
#include <cstdint>

#include "conversions.h"


StepIntervals calculateStepIntervals(StepCount steps)
{
    StepIntervals result{0, 0};

    if (steps.x == 0 && steps.y == 0) {
        return result;
    }

    uint32_t maxSteps = std::max(steps.x, steps.y);
    uint32_t minSteps = std::min(steps.x, steps.y);

    double time = static_cast<double>(maxSteps) * Config::STEP_INTERVAL_US;

    double smallerInterval = (minSteps > 0) 
        ? time / minSteps
        : 0.0;

    if (steps.x >= steps.y) {
        result.x = static_cast<uint32_t>(Config::STEP_INTERVAL_US);
        result.y = static_cast<uint32_t>(smallerInterval);
    } else {
        result.x = static_cast<uint32_t>(smallerInterval);
        result.y = static_cast<uint32_t>(Config::STEP_INTERVAL_US);
    }

    return result;
}


StepCount calculateStepCount(uint32_t dx, uint32_t dy)
{
    return StepCount {
        static_cast<uint32_t>(dx * Config::MM_TO_STEP),
        static_cast<uint32_t>(dy * Config::MM_TO_STEP)
    };
}