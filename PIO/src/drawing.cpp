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

  Serial.println("[LINE] Moving to line start");
  StatusCode status = Motion::moveTo(start);
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

  StatusCode status_1 = Motion::moveTo(Point {static_cast<int16_t>(topLeft.x + width), static_cast<int16_t>(topLeft.y)});
  if (status_1 != StatusCode::OK) {
    Serial.println("[SQUARE] Move to top right failed");
    return status_1;
  }

  delay(250);

  StatusCode status_2 = Motion::moveTo(Point {static_cast<int16_t>(topLeft.x + width), static_cast<int16_t>(topLeft.y + height)});
  if (status_2 != StatusCode::OK) {
    Serial.println("[SQUARE] Move to bottom right failed");
    return status_2;
  }

  delay(250);

  StatusCode status_3 = Motion::moveTo(Point {static_cast<int16_t>(topLeft.x), static_cast<int16_t>(topLeft.y + height)});
  if (status_3 != StatusCode::OK) {
    Serial.println("[SQUARE] Move to bottom left failed");
    return status_3;
  }

  delay(250);

  StatusCode status_4 = Motion::moveTo(topLeft);
  if (status_4 != StatusCode::OK) {
    Serial.println("[SQUARE] Move back to start failed");
    return status_4;
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

  StatusCode status = arc(center, radius, 0, 359);
  if (status != StatusCode::OK) {
    Serial.println("[CIRCLE] Drawing circle failed");
  }

  Serial.println("[CIRCLE] Template complete");
  return status;
}

StatusCode arc(const Point &center, int16_t radius, int16_t startAngle,
               int16_t endAngle) {
  Serial.println("[ARC] Start command");
  if (radius < 0) {
    Serial.println("[ARC] Invalid radius");
    return StatusCode::ERR_RANGE;
  }

  float start_rad = startAngle * Config::OUR_PI / 180.0;
  int32_t start_x = center.x + radius * cos(start_rad);
  int32_t start_y = center.y + radius * sin(start_rad);

  Point last = {static_cast<int16_t>(start_x), static_cast<int16_t>(start_y)};

  StatusCode status = Motion::moveTo(last);
  if (status != StatusCode::OK) {
    Serial.println("[ARC] Move to start failed");
    return status;
  }

  delay(250);

  Motion::penDown();

  delay(1000);

  float stepRad = 2.0f * asin(Config::ARC_SEGMENT_LEN / (2.0f * radius));
  float stepDeg = stepRad * 180.0f / Config::OUR_PI;
  float arcStep = max(0.1f, stepDeg);

  for (float a = startAngle + arcStep; a <= endAngle; a += arcStep) {
    float rad = a * Config::OUR_PI / 180.0f;
    Point next = {
        static_cast<int16_t>(lround(center.x + radius * cos(rad))),
        static_cast<int16_t>(lround(center.y + radius * sin(rad)))
    };

    if (next.x == last.x && next.y == last.y) continue;

    status = Motion::smoothMove(next);
    if (status != StatusCode::OK) {
        Serial.println("[ARC] Incremental move failed");
        return status;
    }
    last = next;
  }

  float end_rad = endAngle * static_cast<float>(Config::OUR_PI) / 180.0f;
  Point end_point = {static_cast<int16_t>(lround(center.x + radius * cos(static_cast<double>(end_rad)))),
                     static_cast<int16_t>(lround(center.y + radius * sin(static_cast<double>(end_rad))))};
  if (last.x != end_point.x || last.y != end_point.y) {
    status = Motion::smoothMove(end_point);
    if (status != StatusCode::OK) {
      Serial.println("[ARC] Move to arc end failed");
      return status;
    }
  }

  delay(250);

  Motion::penUp();
  Motion::home();

  Serial.println("[ARC] Template complete");
  return StatusCode::OK;
}

StatusCode logo() {
  Serial.println("[LOGO] Printing competition logo");
  Motion::home();
  line(Point {40, 15}, Point {30, 15});
  line(Point {30, 15}, Point {30, 20});
  line(Point {30, 20}, Point {40, 20});
  line(Point {40, 20}, Point {40, 25});
  line(Point {40, 25}, Point {30, 25});
  line(Point {43, 15}, Point {53, 15});
  line(Point {48, 15}, Point {48, 25});
  line(Point {56, 15}, Point {66, 15});
  line(Point {66, 15}, Point {66, 25});
  line(Point {66, 25}, Point {56, 25});
  line(Point {56, 25}, Point {56, 15});
  line(Point {69, 25}, Point {69, 15});
  line(Point {69, 15}, Point {79, 25});
  line(Point {79, 25}, Point {79, 15});
  line(Point {82, 15}, Point {92, 15});
  line(Point {82, 15}, Point {82, 25});
  line(Point {82, 25}, Point {92, 25});
  line(Point {82, 20}, Point {90, 20});
  line(Point {95, 25}, Point {95, 15});
  line(Point {95, 15}, Point {105, 15});
  line(Point {105, 15}, Point {105, 20});
  line(Point {105, 20}, Point {95, 20});
  line(Point {100, 20}, Point {105, 25});
  line(Point {108, 15}, Point {118, 15});
  line(Point {113, 015}, Point {113, 025});
  line(Point {108, 025}, Point {118, 025});
  line(Point {121, 015}, Point {121, 025});
  line(Point {121, 025}, Point {129, 025});
  line(Point {129, 025}, Point {133, 022});
  line(Point {133, 022}, Point {133, 18});
  line(Point {133, 18}, Point {129, 015});
  line(Point {129, 015}, Point {121, 015});
  line(Point {146, 015}, Point {136, 015});
  line(Point {136, 015}, Point {136, 025});
  line(Point {136, 025}, Point {146, 025});
  line(Point {146, 025}, Point {146, 020});
  line(Point {146, 020}, Point {138, 020});
  line(Point {149, 015}, Point {159, 015});
  line(Point {149, 015}, Point {149, 025});
  line(Point {149, 025}, Point {159, 025});
  line(Point {149, 020}, Point {157, 020});
  Motion::home();
  line(Point {155, 45}, Point {60, 114});
  circle(Point {65, 123}, 10);
  line(Point {57, 130}, Point {90, 180});
  line(Point {90, 180}, Point {14, 250});
  line(Point {14, 250}, Point {90, 193});
  line(Point {90, 193}, Point {26, 258});
  line(Point {26, 258}, Point {100, 200});
  line(Point {100, 200}, Point {43, 266});
  line(Point {43, 266}, Point {137, 182});
  circle(Point {130, 175}, 10);
  line(Point {139, 170}, Point {113, 130});
  line(Point {113, 130}, Point {184, 61});
  line(Point {184, 61}, Point {107, 114});
  line(Point {107, 114}, Point {170, 53});
  line(Point {170, 53}, Point {97, 102});
  line(Point {97, 102}, Point {155, 45});
  Motion::home();
  Serial.println("[LOGO] Competition logo printed");
  return StatusCode::OK;
}
}  // namespace Drawing
