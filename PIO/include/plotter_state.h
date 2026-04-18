#ifndef PLOTTER_STATE_H
#define PLOTTER_STATE_H

#include "types.h"

namespace PlotterState {
void init();
Point getPosition();
void setPosition(const Point &point);
bool getPenDown();
void setPenDown(bool isDown);
}  // namespace PlotterState

#endif
