#include "motion.h"

#include <Arduino.h>
#include <Servo.h>

#include "config.h"
#include "plotter_state.h"
#include "stepperMotor.h"

namespace {
Servo g_penServo;
bool g_servoAttached = false;
}  // namespace

namespace Motion {
void init() {
  Serial.println("[INIT] Initializing stepper driver");
  Stepper_Init();
  pinMode(Config::M1_ENB, OUTPUT);
  pinMode(Config::M2_ENB, OUTPUT);

  // DRV8825 enable pin is active LOW.
  digitalWrite(Config::M1_ENB, LOW);
  digitalWrite(Config::M2_ENB, LOW);

  Serial.print("[INIT] Attaching pen servo on pin ");
  Serial.println(Config::SERVO_PIN);
  g_penServo.attach(Config::SERVO_PIN);
  g_servoAttached = true;
  penUp();
  Serial.println("[INIT] Motion subsystem ready");
}

bool isPointInRange(const Point &point) {
  return point.x >= Config::X_MIN && point.x <= Config::X_MAX &&
         point.y >= Config::Y_MIN && point.y <= Config::Y_MAX;
}

StatusCode home() {
  Serial.println("[HOME] Start homing to 0,0");
  // TODO: Replace this with real homing using endstops.
  PlotterState::setPosition({0, 0});
  penDown();
  delay(1000);
  penUp();
  Serial.println("[HOME] Homing complete");
  return StatusCode::OK;
}

StatusCode moveTo(const Point &target) {
  Serial.print("[MOVE] Request to ");
  Serial.print(target.x);
  Serial.print(",");
  Serial.println(target.y);

  if (!isPointInRange(target)) {
    Serial.println("[MOVE] Target out of range");
    return StatusCode::ERR_RANGE;
  }

  // TODO: Convert XY millimeter grid to stepper steps and drive both motors.
  PlotterState::setPosition(target);
  Serial.println("[MOVE] Target accepted (template move)");
  return StatusCode::OK;
}

void penUp() {
  if (g_servoAttached) {
    g_penServo.write(Config::PEN_UP_ANGLE);
  }
  PlotterState::setPenDown(false);
  Serial.println("[PEN] Pen UP");
}

void penDown() {
  if (g_servoAttached) {
    g_penServo.write(Config::PEN_DOWN_ANGLE);
  }
  PlotterState::setPenDown(true);
  Serial.println("[PEN] Pen DOWN");
}
}  // namespace Motion
