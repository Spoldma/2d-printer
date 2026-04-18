#include <Arduino.h>

#include "command_dispatcher.h"
#include "command_parser.h"
#include "config.h"
#include "motion.h"
#include "plotter_state.h"
#include "web_interface.h"

namespace {
String g_serialLine;

const char *statusToText(StatusCode code) {
  switch (code) {
    case StatusCode::OK:
      return "OK";
    case StatusCode::ERR_PARSE:
      return "ERR_PARSE";
    case StatusCode::ERR_RANGE:
      return "ERR_RANGE";
    case StatusCode::ERR_UNSUPPORTED:
      return "ERR_UNSUPPORTED";
    default:
      return "ERR_UNKNOWN";
  }
}

StatusCode processLine(const String &line, const char *sourceTag) {
  Serial.print(sourceTag);
  Serial.print(" RX: ");
  Serial.println(line);
  Command cmd = {};
  StatusCode parseStatus = CommandParser::parse(line, cmd);
  if (parseStatus != StatusCode::OK) {
    Serial.print(sourceTag);
    Serial.print(" ");
    Serial.println(statusToText(parseStatus));
    return parseStatus;
  }

  StatusCode execStatus = CommandDispatcher::execute(cmd);
  Serial.print(sourceTag);
  Serial.print(" ");
  Serial.println(statusToText(execStatus));
  return execStatus;
}

}  // namespace

void setup() {
  Serial.begin(Config::SERIAL_BAUD_RATE);
  PlotterState::init();
  Motion::init();
  WebInterface::begin(processLine);

  Serial.println("2D plotter template ready.");
}

void loop() {
  WebInterface::handleClient();

  while (Serial.available() > 0) {
    char ch = static_cast<char>(Serial.read());
    if (ch == '\r') {
      continue;
    }

    if (ch == '\n') {
      if (g_serialLine.length() > 0) {
        processLine(g_serialLine, "[SERIAL]");
        g_serialLine = "";
      }
    } else {
      g_serialLine += ch;
    }
  }
}