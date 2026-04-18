#ifndef DRAWING_H
#define DRAWING_H

#include "types.h"

namespace Drawing {
StatusCode dot(const Point &at);
StatusCode line(const Point &start, const Point &end);
StatusCode square(const Point &topLeft, int16_t width, int16_t height);
StatusCode circle(const Point &center, int16_t radius);
StatusCode arc(const Point &center, int16_t radius, int16_t startAngle,
               int16_t endAngle);
StatusCode logo();
}  // namespace Drawing

#endif
