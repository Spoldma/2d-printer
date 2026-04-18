#include "command_parser.h"

#include <stdlib.h>
#include <string.h>

namespace {
bool parseIntList(const String &input, int32_t *outValues, uint8_t &outCount,
                  uint8_t maxValues) {
  outCount = 0;
  int start = 0;
  while (start <= input.length() && outCount < maxValues) {
    int comma = input.indexOf(',', start);
    String token = (comma >= 0) ? input.substring(start, comma)
                                : input.substring(start);
    token.trim();
    if (token.length() == 0) {
      return false;
    }

    char *endPtr = nullptr;
    long value = strtol(token.c_str(), &endPtr, 10);
    if (*endPtr != '\0') {
      return false;
    }

    outValues[outCount++] = static_cast<int32_t>(value);
    if (comma < 0) {
      break;
    }
    start = comma + 1;
  }

  if (start < input.length() && input.indexOf(',', start) >= 0 &&
      outCount == maxValues) {
    return false;
  }

  return outCount > 0;
}

bool parseWithPrefix(const String &line, const char *prefix, uint8_t expectedArgs,
                     CommandType type, Command &outCommand) {
  if (!line.startsWith(prefix)) {
    return false;
  }

  String argPart = line.substring(strlen(prefix));
  uint8_t count = 0;
  if (!parseIntList(argPart, outCommand.args, count, 5)) {
    return false;
  }
  if (count != expectedArgs) {
    return false;
  }

  outCommand.type = type;
  outCommand.argCount = count;
  return true;
}
}  // namespace

namespace CommandParser {
StatusCode parse(const String &inputLine, Command &outCommand) {
  String line = inputLine;
  line.trim();
  line.toUpperCase();

  outCommand.type = CommandType::INVALID;
  outCommand.argCount = 0;
  for (uint8_t i = 0; i < 5; ++i) {
    outCommand.args[i] = 0;
  }

  if (line.length() == 0) {
    return StatusCode::ERR_PARSE;
  }

  if (line == "HOME") {
    outCommand.type = CommandType::HOME;
    return StatusCode::OK;
  }
  if (line == "LOGO") {
    outCommand.type = CommandType::LOGO;
    return StatusCode::OK;
  }
  if (parseWithPrefix(line, "MOV:", 2, CommandType::MOV, outCommand)) {
    return StatusCode::OK;
  }
  if (parseWithPrefix(line, "DOT:", 2, CommandType::DOT, outCommand)) {
    return StatusCode::OK;
  }
  if (parseWithPrefix(line, "LINE:", 4, CommandType::LINE, outCommand)) {
    return StatusCode::OK;
  }
  if (parseWithPrefix(line, "SQUARE:", 4, CommandType::SQUARE, outCommand)) {
    return StatusCode::OK;
  }
  if (parseWithPrefix(line, "CIRCLE:", 3, CommandType::CIRCLE, outCommand)) {
    return StatusCode::OK;
  }
  if (parseWithPrefix(line, "ARC:", 5, CommandType::ARC, outCommand)) {
    return StatusCode::OK;
  }

  return StatusCode::ERR_PARSE;
}
}  // namespace CommandParser
