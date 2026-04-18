#include "plotter_state.h"

namespace {
Point g_position = {0, 0};
bool g_penDown = false;
}  // namespace

namespace PlotterState {
void init() {
  g_position = {0, 0};
  g_penDown = false;
}

Point getPosition() { return g_position; }

void setPosition(const Point &point) { g_position = point; }

bool getPenDown() { return g_penDown; }

void setPenDown(bool isDown) { g_penDown = isDown; }
}  // namespace PlotterState
