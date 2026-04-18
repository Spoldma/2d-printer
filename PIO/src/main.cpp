#include <Arduino.h>
#include "stepperMotor.h"

// MOTOR 1
#define M1_ENB 9
#define M1_STEP 7
#define M1_DIR 8

// MOTOR 2
#define M2_ENB 10
#define M2_STEP 5
#define M2_DIR 6

// Assuming 1/8 microstepping with 1.8 degree steppers.
static const uint32_t STEPS_PER_REVOLUTION = 1600;
static const uint32_t STEP_INTERVAL_US = 800;
static const uint32_t PAUSE_MS = 1000;

void setup() {
  Serial.begin(115200);
  Stepper_Init();
  pinMode(LED_BUILTIN, OUTPUT);

  // DRV8825 enable pin is active LOW.
  //digitalWrite(M1_ENB, LOW);
  //digitalWrite(M2_ENB, LOW);

  Serial.println("Stepper direction test started.");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("Phase 1: motor DIR=1");
  Stepper_StartNonBlocking(MOTOR_1, STEP_INTERVAL_US, 1, STEPS_PER_REVOLUTION);
  while(Stepper_IsBusy());
  
  delay(1000);

  Serial.println("Phase 2: motors DIR=0");
  Stepper_StartNonBlocking(MOTOR_1, STEP_INTERVAL_US, 0, STEPS_PER_REVOLUTION);
  while(Stepper_IsBusy());

  delay(1000);

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}