#ifndef PLOTTER_STATE_H
#define PLOTTER_STATE_H

#include "types.h"

namespace PlotterState {
void init();
Point getPosition();
void setPosition(const Point &point);
/// Advance the logical position by the physical motion implied by motor steps.
void addPhysicalDelta(double dx_mm, double dy_mm);
bool getPenDown();
void setPenDown(bool isDown);
}  // namespace PlotterState

#endif
