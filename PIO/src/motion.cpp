#include "motion.h"

#include <Arduino.h>
#include <Servo.h>

#include "config.h"
#include "plotter_state.h"
#include "stepperMotor.h"
#include "conversions.h"

namespace {
Servo g_penServo;
bool g_servoAttached = false;

bool isSwitchPressed(uint8_t pin) {
  // Limit switches are wired as INPUT_PULLUP, so LOW means pressed.
  return digitalRead(pin) == LOW;
}

StatusCode homeAxis(stepMotor_Id motor, uint8_t switchPin, uint8_t towardSwitchDir,
                    const char *axisName) {
  Serial.print("[HOME] ");
  Serial.print(axisName);
  Serial.println(" axis: start");

  const uint8_t awayFromSwitchDir = towardSwitchDir ? 0 : 1;

  // If already on the switch, back off first to guarantee a clean trigger.
  if (isSwitchPressed(switchPin)) {
    Serial.print("[HOME] ");
    Serial.print(axisName);
    Serial.println(" axis: switch already pressed, backing off");
    Stepper_MoveBlocking(motor, Config::HOME_STEP_INTERVAL_US, awayFromSwitchDir,
                         Config::HOME_BACKOFF_STEPS);
  }

  uint32_t movedSteps = 0;
  while (!isSwitchPressed(switchPin) &&
         movedSteps < Config::HOME_MAX_STEPS_PER_AXIS) {
    Stepper_MoveBlocking(motor, Config::HOME_STEP_INTERVAL_US, towardSwitchDir, 1);
    movedSteps++;
  }

  if (!isSwitchPressed(switchPin)) {
    Serial.print("[HOME] ");
    Serial.print(axisName);
    Serial.println(" axis: FAILED (switch not reached)");
    return StatusCode::ERR_RANGE;
  }

  delay(500);

  Serial.print("[HOME] ");
  Serial.print(axisName);
  Serial.println(" axis: switch reached, backing off");
  Stepper_MoveBlocking(motor, Config::HOME_STEP_INTERVAL_US, awayFromSwitchDir,
                       Config::HOME_BACKOFF_STEPS);

  Serial.print("[HOME] ");
  Serial.print(axisName);
  Serial.println(" axis: done");
  return StatusCode::OK;
}
}  // namespace

namespace Motion {
void init() {
  Serial.println("[INIT] Initializing stepper driver");
  Stepper_Init();

  Serial.print("[INIT] Attaching pen servo on pin ");
  Serial.println(Config::SERVO_PIN);
  g_penServo.attach(Config::SERVO_PIN);
  g_servoAttached = true;
  penUp();

  pinMode(Config::X_LIMIT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(Config::Y_LIMIT_SWITCH_PIN, INPUT_PULLUP);
  Serial.print("[INIT] X limit pin: ");
  Serial.println(Config::X_LIMIT_SWITCH_PIN);
  Serial.print("[INIT] Y limit pin: ");
  Serial.println(Config::Y_LIMIT_SWITCH_PIN);
  Serial.println("[INIT] Motion subsystem ready");
}

bool isPointInRange(const Point &point) {
  return point.x >= Config::X_MIN && point.x <= Config::X_MAX &&
         point.y >= Config::Y_MIN && point.y <= Config::Y_MAX;
}

StatusCode home() {
  Serial.println("[HOME] Start homing to 0,0");
  penUp();

  StatusCode xStatus = homeAxis(MOTOR_1, Config::X_LIMIT_SWITCH_PIN,
                                Config::HOME_X_TOWARD_SWITCH_DIR, "X");
  if (xStatus != StatusCode::OK) {
    return xStatus;
  }

  StatusCode yStatus = homeAxis(MOTOR_2, Config::Y_LIMIT_SWITCH_PIN,
                                Config::HOME_Y_TOWARD_SWITCH_DIR, "Y");
  if (yStatus != StatusCode::OK) {
    return yStatus;
  }

  PlotterState::setPosition({0, 0});
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

  Point current = PlotterState::getPosition();
  int32_t dx = static_cast<int32_t>(target.x) - static_cast<int32_t>(current.x);
  int32_t dy = static_cast<int32_t>(target.y) - static_cast<int32_t>(current.y);

  StepCount steps = calculateStepCount(abs(dx), abs(dy));
  StepIntervals intervals = calculateStepIntervals(steps);

  uint8_t xDir = (dx >= 0) ? 1 : 0;
  uint8_t yDir = (dy >= 0) ? 1 : 0;

  Serial.print("[MOVE] Current ");
  Serial.print(current.x);
  Serial.print(",");
  Serial.print(current.y);
  Serial.print(" | Delta ");
  Serial.print(dx);
  Serial.print(",");
  Serial.println(dy);

  if (steps.x == 0 && steps.y == 0) {
    Serial.println("[MOVE] Already at target");
    return StatusCode::OK;
  }

  // Start both axes with non-blocking calls so they run in parallel.
  if (steps.x > 0) {
    Serial.print("[MOVE] MOTOR_1 start dir=");
    Serial.print(xDir);
    Serial.print(" steps=");
    Serial.println(steps.x);
    Stepper_StartNonBlocking(MOTOR_1, intervals.x, xDir, steps.x);
  }
  if (steps.y > 0) {
    Serial.print("[MOVE] MOTOR_2 start dir=");
    Serial.print(yDir);
    Serial.print(" steps=");
    Serial.println(steps.y);
    Stepper_StartNonBlocking(MOTOR_2, intervals.y, yDir, steps.y);
  }

  while (Stepper_IsBusy()) {
    delay(1);
  }

  PlotterState::setPosition(target);
  Serial.println("[MOVE] Motion complete");
  return StatusCode::OK;
}

StatusCode smoothMove(const Point &target) {
    Point current = PlotterState::getPosition();

    Serial.print("[MOVE] Request to ");
    Serial.print(target.x);
    Serial.print(",");
    Serial.println(target.y);

    if (!isPointInRange(target)) {
      Serial.println("[MOVE] Target out of range");
      return StatusCode::ERR_RANGE;
    }

    int dx = abs(target.x - current.x);
    int dy = abs(target.y - current.y);

    int sx = (target.x >= current.x) ? 1 : -1;
    int sy = (target.y >= current.y) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        if (current.x == target.x && current.y == target.y)
            break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            current.x += sx;
            Stepper_stepOnceWithDir(MOTOR1, sx);
        }

        if (e2 < dx) {
            err += dx;
            current.y += sy;
            Stepper_stepOnceWithDir(MOTOR2, sy);
        }

        // control speed here
        delayMicroseconds(Config::STEP_INTERVAL_US);
    }

    PlotterState::setPosition(target);
    Serial.println("[MOVE] Motion complete");
    return StatusCode::OK;
}

StatusCode smoothMove(const Point &target) {
    Point current = PlotterState::getPosition();

    Serial.print("[MOVE] Request to ");
    Serial.print(target.x);
    Serial.print(",");
    Serial.println(target.y);

    if (!isPointInRange(target)) {
      Serial.println("[MOVE] Target out of range");
      return StatusCode::ERR_RANGE;
    }

    int dx = abs(target.x - current.x);
    int dy = abs(target.y - current.y);

    int sx = (target.x >= current.x) ? 1 : -1;
    int sy = (target.y >= current.y) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        if (current.x == target.x && current.y == target.y)
            break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            current.x += sx;
            Stepper_stepOnceWithDir(MOTOR1, sx);
        }

        if (e2 < dx) {
            err += dx;
            current.y += sy;
            Stepper_stepOnceWithDir(MOTOR2, sy);
        }

        // control speed here
        delayMicroseconds(Config::STEP_INTERVAL_US);
    }

    PlotterState::setPosition(target);
    Serial.println("[MOVE] Motion complete");
    return StatusCode::OK;
}

StatusCode penUp() {
  if (g_servoAttached) {
    g_penServo.write(Config::PEN_UP_ANGLE);
  }
  PlotterState::setPenDown(false);
  Serial.println("[PEN] Pen UP");
  return StatusCode::OK;
}

StatusCode penDown() {
  if (g_servoAttached) {
    g_penServo.write(Config::PEN_DOWN_ANGLE);
  }
  PlotterState::setPenDown(true);
  Serial.println("[PEN] Pen DOWN");
  return StatusCode::OK;
}
}  // namespace Motion
