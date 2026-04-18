#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <Arduino.h>

#include "types.h"

namespace CommandParser {
StatusCode parse(const String &line, Command &outCommand);
}  // namespace CommandParser

#endif
