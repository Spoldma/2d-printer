#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

enum class StatusCode : uint8_t {
  OK = 0,
  ERR_PARSE,
  ERR_RANGE,
  ERR_UNSUPPORTED
};

enum class CommandType : uint8_t {
  HOME = 0,
  MOV,
  DOT,
  LINE,
  SQUARE,
  CIRCLE,
  ARC,
  LOGO,
  INVALID
};

struct Point {
  int16_t x;
  int16_t y;
};

struct Command {
  CommandType type;
  int32_t args[5];
  uint8_t argCount;
};

#endif
