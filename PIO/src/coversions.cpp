#include <algorithm>
#include <cstdint>

#include "conversions.h"


StepIntervals calculateStepIntervals(uint32_t dx, uint32_t dy)
{
    StepIntervals result{0, 0};

    if (dx == 0 && dy == 0) {
        return result;
    }

    uint32_t maxSteps = std::max(dx, dy);
    uint32_t minSteps = std::min(dx, dy);

    double time = static_cast<double>(maxSteps) * Config::STEP_INTERVAL_US;

    double smallerInterval = (minSteps > 0) 
        ? time / minSteps
        : 0.0;

    if (dx >= dy) {
        result.x = static_cast<uint32_t>(Config::STEP_INTERVAL_US);
        result.y = static_cast<uint32_t>(smallerInterval);
    } else {
        result.x = static_cast<uint32_t>(smallerInterval);
        result.y = static_cast<uint32_t>(Config::STEP_INTERVAL_US);
    }

    return result;
}