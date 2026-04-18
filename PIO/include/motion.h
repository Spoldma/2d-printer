#ifndef MOTION_H
#define MOTION_H

#include "types.h"

namespace Motion {
void init();
StatusCode home();
StatusCode moveTo(const Point &target);
StatusCode smoothMove(const Point &target);
bool isPointInRange(const Point &point);
void penUp();
void penDown();
}  // namespace Motion

#endif
