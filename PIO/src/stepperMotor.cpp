#include "stepperMotor.h"

#include <Arduino.h>
#include <esp_timer.h>

#include "config.h"

typedef struct {
  uint8_t dirPin;
  uint8_t stepPin;
  uint8_t enablePin;
  esp_timer_handle_t timer;
} stepMotorConf_T;

typedef struct {
  volatile uint32_t step_target;
  volatile uint32_t curr_step;
  volatile bool isIdle;
  volatile bool stepLevel;
} stepMotorState_T;

static void stepper_timer_cb(void *arg);

/* Configuration structure for the steppers. */
static stepMotorConf_T stepper_conf[NUMBER_OF_STEPPER_MOTORS] = {
    {.dirPin = Config::M1_DIR,
     .stepPin = Config::M1_STEP,
     .enablePin = Config::M1_ENB,
     .timer = nullptr}, /* MOTOR_1 */
    {.dirPin = Config::M2_DIR,
     .stepPin = Config::M2_STEP,
     .enablePin = Config::M2_ENB,
     .timer = nullptr} /* MOTOR_2 */
};

static volatile stepMotorState_T stepper_state[NUMBER_OF_STEPPER_MOTORS];

/**
 * @brief Initialize all stepper motors and associated timers.
 *
 * Configures GPIO pins, initializes timer instances, and prepares
 * internal state structures. Must be called once at startup before
 * using any other stepper functions.
 */
void Stepper_Init(void)
{
  for (int x = 0; x < NUMBER_OF_STEPPER_MOTORS; x++) {
    volatile stepMotorState_T * state_ptr = &stepper_state[x];
    stepMotorConf_T * conf_ptr = &stepper_conf[x];

    state_ptr->step_target = 0;
    state_ptr->curr_step = 0;
    state_ptr->isIdle = true;
    state_ptr->stepLevel = false;

    pinMode(conf_ptr->dirPin, OUTPUT);
    pinMode(conf_ptr->stepPin, OUTPUT);
    pinMode(conf_ptr->enablePin, OUTPUT);
    digitalWrite(conf_ptr->stepPin, LOW);
    digitalWrite(conf_ptr->enablePin, HIGH);

    if (conf_ptr->timer == nullptr) {
      esp_timer_create_args_t timerArgs = {};
      timerArgs.callback = &stepper_timer_cb;
      timerArgs.arg = reinterpret_cast<void *>(static_cast<uintptr_t>(x));
      timerArgs.dispatch_method = ESP_TIMER_TASK;
      timerArgs.name = (x == MOTOR_1) ? "stepper1" : "stepper2";
      esp_timer_create(&timerArgs, &conf_ptr->timer);
    }
  }
}

/**
 * @brief Start a stepper motor using timer-driven interrupts.
 *
 * This function initiates non-blocking motion. The motor will step
 * at the specified interval until the target number of steps is reached.
 *
 * @param motor    Stepper motor ID
 * @param interval Time between full steps in microseconds
 * @param dir      Direction of motion (0 = one direction, 1 = opposite)
 * @param target   Number of steps to execute
 */
void Stepper_StartNonBlocking(stepMotor_Id motor, uint32_t interval, uint8_t dir, uint32_t target)
{
  if (motor < NUMBER_OF_STEPPER_MOTORS) {
    const stepMotorConf_T * conf_ptr = &stepper_conf[motor];

    if (conf_ptr->timer == nullptr) {
      return;
    }

    if (interval < 2) {
      interval = 2;
    }

    if (target == 0) {
      stepper_state[motor].isIdle = true;
      return;
    }

    if (stepper_state[motor].isIdle == true) {
      digitalWrite(conf_ptr->stepPin, LOW);
      stepper_state[motor].stepLevel = false;

      digitalWrite(conf_ptr->dirPin, dir ? HIGH : LOW);
      digitalWrite(conf_ptr->enablePin, LOW);

      stepper_state[motor].step_target = target;
      stepper_state[motor].curr_step = 0;
      stepper_state[motor].isIdle = false;

      esp_timer_stop(conf_ptr->timer);
      esp_timer_start_periodic(conf_ptr->timer, interval / 2);
    }
  }
}

/**
 * @brief Stop a running stepper motor.
 *
 * Stops the associated timer, marks the motor as idle, and disables
 * the motor driver.
 *
 * @param motor Stepper motor ID
 */
void Stepper_Stop(stepMotor_Id motor)
{
  if (motor < NUMBER_OF_STEPPER_MOTORS) {
    const stepMotorConf_T * conf_ptr = &stepper_conf[motor];
    if (conf_ptr->timer != nullptr) {
      esp_timer_stop(conf_ptr->timer);
    }
    stepper_state[motor].isIdle = true;
    stepper_state[motor].stepLevel = false;
    digitalWrite(conf_ptr->stepPin, LOW);
    digitalWrite(conf_ptr->enablePin, HIGH);
  }
}

/**
 * @brief Perform a blocking stepper motor movement.
 *
 * Spins the motor for a specified number of steps. This function blocks
 * execution until all steps are completed, and therefore cannot be used
 * for simultaneous multi-motor control.
 *
 * @param motor            Stepper motor ID
 * @param interval         Time between full steps in microseconds
 * @param dir              Direction of motion (0 = one direction, 1 = opposite)
 * @param number_of_steps  Total number of steps to execute
 */
void Stepper_MoveBlocking(stepMotor_Id motor, uint32_t interval, uint8_t dir, uint32_t number_of_steps)
{
  if (motor < NUMBER_OF_STEPPER_MOTORS) {
    const stepMotorConf_T * conf_ptr = &stepper_conf[motor];

    if (stepper_state[motor].isIdle == true) {
      digitalWrite(conf_ptr->dirPin, dir ? HIGH : LOW);

      digitalWrite(conf_ptr->enablePin, LOW);

      // Spin the stepper motor with blocking step pulses.
      for (uint32_t i = 0; i < number_of_steps; i++) {
        Stepper_StepOnce(motor, interval);
      }

      digitalWrite(conf_ptr->enablePin, HIGH);
    }
  }
}

/**
 * @brief Execute a single step on the stepper motor.
 *
 * Generates one step pulse using a blocking delay. The enable pin must
 * be managed externally before and after calling this function.
 *
 * @param motor   Stepper motor ID
 * @param interval Time between full steps in microseconds
 */
void Stepper_StepOnce(stepMotor_Id motor, int interval)
{
  const stepMotorConf_T * conf_ptr = &stepper_conf[motor];

  // These four lines result in 1 full step pulse.
  digitalWrite(conf_ptr->stepPin, HIGH);
  delayMicroseconds(interval / 2);
  digitalWrite(conf_ptr->stepPin, LOW);
  delayMicroseconds(interval / 2);
}

void Stepper_StepOnceWithDir(stepMotor_Id motor, int8_t dir)
{
  const stepMotorConf_T * conf_ptr = &stepper_conf[motor];

  digitalWrite(conf_ptr->dirPin, dir > 0 ? HIGH : LOW);

  digitalWrite(conf_ptr->stepPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(conf_ptr->stepPin, LOW);
}

/**
 * @brief Check if any stepper motor is currently active.
 *
 * @return true  At least one motor is running
 * @return false All motors are idle
 */
bool Stepper_IsBusy(void)
{ 
  bool res = false;
  for (int x = 0; x < NUMBER_OF_STEPPER_MOTORS; x++) {
    if (stepper_state[x].isIdle == false) {
      res = true;
    }
  }

  return res;
}

static void stepper_timer_cb(void *arg)
{
  const stepMotor_Id motor = static_cast<stepMotor_Id>(reinterpret_cast<uintptr_t>(arg));
  const stepMotorConf_T * conf_ptr = &stepper_conf[motor];
  volatile stepMotorState_T * state_ptr = &stepper_state[motor];

  if (state_ptr->isIdle) {
    return;
  }

  state_ptr->stepLevel = !state_ptr->stepLevel;
  digitalWrite(conf_ptr->stepPin, state_ptr->stepLevel ? HIGH : LOW);

  // Count only rising edge as one step.
  if (state_ptr->stepLevel) {
    state_ptr->curr_step++;
  }

  if (state_ptr->curr_step >= state_ptr->step_target) {
    esp_timer_stop(conf_ptr->timer);
    state_ptr->isIdle = true;
    state_ptr->stepLevel = false;
    digitalWrite(conf_ptr->stepPin, LOW);
    digitalWrite(conf_ptr->enablePin, HIGH);
  }
}
