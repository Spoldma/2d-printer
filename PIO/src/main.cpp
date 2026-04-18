#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);


#include "stepperMotor.h"
/* For the servo we use a built-in header file Servo.h */
#include <Servo.h>

// MOTOR 1
#define M1_ENB 9
#define M1_STEP 7
#define M1_DIR 8

// MOTOR 2
#define M2_ENB 10
#define M2_STEP 5
#define M2_DIR 6

#define SERVO_EXTENDED 0
#define SERVO_RETRACTED 90

/* Assuming 1/8 microstepping. 1.8 degrees for 1 full step.
So a total of 200 full steps for 360 degrees, multiplied by 8, we get 1600 */
#define STEPS_PER_REVOLUTION 1600

static int direction = 0;
static bool isInitComplete = false;
Servo myServo;

void setup() {
  Stepper_Init();

  pinMode(LED_BUILTIN, OUTPUT);

  //enable motors
  digitalWrite(M1_ENB, LOW);
  digitalWrite(M2_ENB, HIGH);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);

  Stepper_MoveBlocking(MOTOR_1, 800, 1, STEPS_PER_REVOLUTION);
  Stepper_MoveBlocking(MOTOR_2, 800, 1, STEPS_PER_REVOLUTION);
  

  delay(1000);

  Stepper_MoveBlocking(MOTOR_1, 800, 0, STEPS_PER_REVOLUTION);
  Stepper_MoveBlocking(MOTOR_2, 800, 0, STEPS_PER_REVOLUTION);

  digitalWrite(LED_BUILTIN, LOW);

  delay(1000);       
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}