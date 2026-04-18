#include <Arduino.h>

#include "command_dispatcher.h"
#include "command_parser.h"
#include "config.h"
#include "motion.h"
#include "plotter_state.h"

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

void processLine(const String &line) {
  Serial.print("[SERIAL] RX: ");
  Serial.println(line);
  Command cmd = {};
  StatusCode parseStatus = CommandParser::parse(line, cmd);
  if (parseStatus != StatusCode::OK) {
    Serial.print("ERR ");
    Serial.println(statusToText(parseStatus));
    return;
  }

  StatusCode execStatus = CommandDispatcher::execute(cmd);
  Serial.println(statusToText(execStatus));
}
}  // namespace

#include <Servo.h>
Servo g_penServo;

void setup() {
  g_penServo.attach(Config::SERVO_PIN);
  g_penServo.write(Config::PEN_UP_ANGLE);
  delay(1000);
  g_penServo.write(Config::PEN_DOWN_ANGLE);

  Serial.begin(Config::SERIAL_BAUD_RATE);
  pinMode(LED_BUILTIN, OUTPUT);

  PlotterState::init();
  Motion::init();

  Serial.println("2D plotter template ready.");
}

void loop() {
  while (Serial.available() > 0) {
    char ch = static_cast<char>(Serial.read());
    if (ch == '\r') {
      continue;
    }

    if (ch == '\n') {
      if (g_serialLine.length() > 0) {
        processLine(g_serialLine);
        g_serialLine = "";
      }
    } else {
      g_serialLine += ch;
    }
  }
}