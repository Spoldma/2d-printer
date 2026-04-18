#include "drawing.h"

#include <Arduino.h>

#include "motion.h"
#include "config.h"

namespace Drawing {
StatusCode dot(const Point &at) {
  Serial.print("[DOT] Start at ");
  Serial.print(at.x);
  Serial.print(",");
  Serial.println(at.y);
  StatusCode moveStatus = Motion::moveTo(at);
  if (moveStatus != StatusCode::OK) {
    Serial.println("[DOT] Move failed");
    return moveStatus;
  }

  Serial.println("[DOT] Creating dot");
  Motion::penDown();
  delay(100);
  Motion::penUp();
  Serial.println("[DOT] Done");
  return StatusCode::OK;
}

StatusCode line(const Point &start, const Point &end) {
  Serial.print("[LINE] Start command from ");
  Serial.print(start.x);
  Serial.print(",");
  Serial.print(start.y);
  Serial.print(" to ");
  Serial.print(end.x);
  Serial.print(",");
  Serial.println(end.y);

  Serial.println("[LINE] Homing before line");
  StatusCode status = Motion::home();
  if (status != StatusCode::OK) {
    Serial.println("[LINE] Homing failed");
    return status;
  }

  Serial.println("[LINE] Moving to line start");
  status = Motion::moveTo(start);
  if (status != StatusCode::OK) {
    Serial.println("[LINE] Move to start failed");
    return status;
  }

  Motion::penDown();
  delay(1000);
  Serial.println("[LINE] Moving to line end");
  status = Motion::moveTo(end);
  Motion::penUp();

  if (status != StatusCode::OK) {
    Serial.println("[LINE] Move to end failed");
    return status;
  }

  Serial.println("[LINE] Done");
  return StatusCode::OK;
}

StatusCode square(const Point &topLeft, int16_t width, int16_t height) {
  Serial.println("[SQUARE] Start command");
  if (width < 0 || height < 0) {
    Serial.println("[SQUARE] Invalid width/height");
    return StatusCode::ERR_RANGE;
  }

  StatusCode status = Motion::moveTo(topLeft);
  if (status != StatusCode::OK) {
    Serial.println("[SQUARE] Move to start failed");
    return status;
  }

  Motion::penDown();

  delay(1000);

  StatusCode status = Motion::moveTo(Point {static_cast<int16_t>(topLeft.x + width), static_cast<int16_t>(topLeft.y)});
  if (status != StatusCode::OK) {
    Serial.println("[SQUARE] Move to top right failed");
    return status;
  }

  delay(250);

  StatusCode status = Motion::moveTo(Point {static_cast<int16_t>(topLeft.x + width), static_cast<int16_t>(topLeft.y + height)});
  if (status != StatusCode::OK) {
    Serial.println("[SQUARE] Move to bottom right failed");
    return status;
  }

  delay(250);

  StatusCode status = Motion::moveTo(Point {static_cast<int16_t>(topLeft.x), static_cast<int16_t>(topLeft.y + height)});
  if (status != StatusCode::OK) {
    Serial.println("[SQUARE] Move to bottom left failed");
    return status;
  }

  delay(250);

  StatusCode status = Motion::moveTo(topLeft);
  if (status != StatusCode::OK) {
    Serial.println("[SQUARE] Move back to start failed");
    return status;
  }

  delay(500);

  Motion::penUp();

  Serial.println("[SQUARE] Template complete");
  return StatusCode::OK;
}

StatusCode circle(const Point &center, int16_t radius) {
  Serial.println("[CIRCLE] Start command");
  if (radius < 0) {
    Serial.println("[CIRCLE] Invalid radius");
    return StatusCode::ERR_RANGE;
  }

  StatusCode status = arc(center, radius, 0, 359)
  if (status != StatusCode::OK) {
    Serial.println("[CIRCLE] Drawing circle failed");
  }

  Serial.println("[CIRCLE] Template complete");
  return status;
}

StatusCode arc(const Point &center, int16_t radius, int16_t startAngle,
               int16_t endAngle) {
  Serial.println("[ARC] Start command");
  (void)startAngle;
  (void)endAngle;
  if (radius < 0) {
    Serial.println("[ARC] Invalid radius");
    return StatusCode::ERR_RANGE;
  }

  float start_rad = startAngle * Config::OUR_PI / 180.0;
  int32_t start_x = center.x + radius * cos(start_rad);
  int32_t start_y = center.y + radius * sin(start_rad);

  StatusCode status = Motion::moveTo(Point {static_cast<int16_t>(start_x), static_cast<int16_t>(start_y)});
  if (status != StatusCode::OK) {
    Serial.println("[ARC] Move to start failed");
    return status;
  }

  delay(250);

  Motion::penDown();

  delay(1000);

  for (float a = startAngle; a <= endAngle; a += Config::ARC_STEP_DEG) {
      float rad = a * Config::OUR_PI / 180.0;
      int32_t x = center.x + radius * cos(rad);
      int32_t y = center.y + radius * sin(rad);

      StatusCode status = Motion::moveTo(Point {static_cast<int16_t>(x), static_cast<int16_t>(y)});
      if (status != StatusCode::OK) {
        Serial.println("[ARC] Incremental move failed");
        return status;
      }
  }

  delay(250);

  Motion::penUp();

  Serial.println("[ARC] Template complete");
  return StatusCode::OK;
}

StatusCode logo() {
  Serial.println("[LOGO] Template not implemented");
  // TODO: Implement competition logo drawing sequence.
  return StatusCode::ERR_UNSUPPORTED;
}
}  // namespace Drawing
