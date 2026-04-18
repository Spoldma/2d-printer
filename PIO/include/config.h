#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

namespace Config {
static const uint32_t SERIAL_BAUD_RATE = 115200;

static const int16_t X_MIN = 0;
static const int16_t X_MAX = 199;
static const int16_t Y_MIN = 0;
static const int16_t Y_MAX = 289;


static const uint8_t M1_ENB = 13;
static const uint8_t M1_STEP = 12;
static const uint8_t M1_DIR = 14;

static const uint8_t M2_ENB = 13;
static const uint8_t M2_STEP = 27;
static const uint8_t M2_DIR = 26;

static const uint8_t SERVO_PIN = 25;
static const uint8_t X_LIMIT_SWITCH_PIN = 32;
static const uint8_t Y_LIMIT_SWITCH_PIN = 33;

static const uint8_t PEN_UP_ANGLE = 90;
static const uint8_t PEN_DOWN_ANGLE = 0;

static const double MM_TO_STEP = 5.3497459859557216536129681510485;

static const uint32_t STEP_INTERVAL_US = 5000;
static const uint32_t HOME_STEP_INTERVAL_US = 3000;
static const uint32_t HOME_MAX_STEPS_PER_AXIS = 20000;
static const uint32_t HOME_BACKOFF_STEPS = 60;
static const uint8_t HOME_X_TOWARD_SWITCH_DIR = 0;
static const uint8_t HOME_Y_TOWARD_SWITCH_DIR = 0;
}  // namespace Config

#endif
