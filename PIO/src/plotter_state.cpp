#include <cmath>

#include "plotter_state.h"

namespace {
// Sub-millimetre position so repeated moves match actual stepping (see
// calculateStepCount truncation in coversions.cpp).
double g_x_mm = 0.0;
double g_y_mm = 0.0;
bool g_penDown = false;
}  // namespace

namespace PlotterState {
void init() {
    g_x_mm = 0.0;
    g_y_mm = 0.0;
    g_penDown = false;
}

Point getPosition() {
    return {static_cast<int16_t>(lround(g_x_mm)),
            static_cast<int16_t>(lround(g_y_mm))};
}

void setPosition(const Point &point) {
    g_x_mm = static_cast<double>(point.x);
    g_y_mm = static_cast<double>(point.y);
}

void addPhysicalDelta(double dx_mm, double dy_mm) {
    g_x_mm += dx_mm;
    g_y_mm += dy_mm;
}

bool getPenDown() { return g_penDown; }

void setPenDown(bool isDown) { g_penDown = isDown; }
}  // namespace PlotterState
