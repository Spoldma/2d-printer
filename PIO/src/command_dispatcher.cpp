#include "command_dispatcher.h"

#include <Arduino.h>

#include "drawing.h"
#include "motion.h"

namespace {
Point toPoint(const int32_t x, const int32_t y) {
  Point point = {static_cast<int16_t>(x), static_cast<int16_t>(y)};
  return point;
}
}  // namespace

namespace CommandDispatcher {
StatusCode execute(const Command &command) {
  Serial.print("[DISPATCH] Command type ");
  Serial.println(static_cast<int>(command.type));
  switch (command.type) {
    case CommandType::HOME:
      return Motion::home();
    case CommandType::MOV:
      return Motion::moveTo(toPoint(command.args[0], command.args[1]));
    case CommandType::DOT:
      return Drawing::dot(toPoint(command.args[0], command.args[1]));
    case CommandType::LINE:
      return Drawing::line(toPoint(command.args[0], command.args[1]),
                           toPoint(command.args[2], command.args[3]));
    case CommandType::SQUARE:
      return Drawing::square(toPoint(command.args[0], command.args[1]),
                             static_cast<int16_t>(command.args[2]),
                             static_cast<int16_t>(command.args[3]));
    case CommandType::CIRCLE:
      return Drawing::circle(toPoint(command.args[0], command.args[1]),
                             static_cast<int16_t>(command.args[2]));
    case CommandType::ARC:
      return Drawing::arc(toPoint(command.args[0], command.args[1]),
                          static_cast<int16_t>(command.args[2]),
                          static_cast<int16_t>(command.args[3]),
                          static_cast<int16_t>(command.args[4]));
    case CommandType::LOGO:
      return Drawing::logo();
    case CommandType::PENUP:
      return Motion::penUp();
    case CommandType::PENDOWN:
      return Motion::penDown();
    default:
      return StatusCode::ERR_UNSUPPORTED;
  }
}
}  // namespace CommandDispatcher
