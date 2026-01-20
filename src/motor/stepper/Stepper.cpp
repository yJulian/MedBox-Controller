#include "Arduino.h"
#include "Stepper.h"

/*
 *   constructor for four-pin version
 *   Sets which wires should control the motor.
 */
Stepper::Stepper(int number_of_steps, int motor_pin_1, int motor_pin_2,
                                      int motor_pin_3, int motor_pin_4)
{
  this->step_number = 0;    // which step the motor is on
  this->direction = 0;      // motor direction
  this->last_step_time = 0; // time stamp in us of the last step taken
  this->number_of_steps = number_of_steps; // total number of steps for this motor

  // Arduino pins for the motor control connection:
  this->motor_pin_1 = motor_pin_1;
  this->motor_pin_2 = motor_pin_2;
  this->motor_pin_3 = motor_pin_3;
  this->motor_pin_4 = motor_pin_4;
}

void Stepper::begin() {
  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_1, OUTPUT);
  pinMode(this->motor_pin_2, OUTPUT);
  pinMode(this->motor_pin_3, OUTPUT);
  pinMode(this->motor_pin_4, OUTPUT);
}

void Stepper::pause() {
  // set all pins to LOW to disable the motor
  digitalWrite(this->motor_pin_1, LOW);
  digitalWrite(this->motor_pin_2, LOW);
  digitalWrite(this->motor_pin_3, LOW);
  digitalWrite(this->motor_pin_4, LOW);
}

void Stepper::resume() {
  // no action needed; motor will step when commanded
}

/*
 * Sets the speed in revs per minute
 */
void Stepper::setSpeed(long whatSpeed)
{
  this->step_delay = 60L * 1000L * 1000L / this->number_of_steps / whatSpeed;
}

/*
 * Moves the motor steps_to_move steps.  If the number is negative,
 * the motor moves in the reverse direction.
 * RTOS friendly version.
 */
void Stepper::step(int steps_to_move)
{
  int steps_left = abs(steps_to_move);  // how many steps to take

  // determine direction based on whether steps_to_mode is + or -:
  if (steps_to_move > 0) { this->direction = 1; }
  if (steps_to_move < 0) { this->direction = 0; }

  while (steps_left > 0)
  {
    const uint32_t now = micros();
    const uint32_t elapsed = (uint32_t)(now - this->last_step_time);

    // move only if the appropriate delay has passed:
    if (elapsed >= this->step_delay)
    {
      internalStep(now, steps_left);
      continue;
    }

    // RTOS-friendly wait (step_delay is in µs; FreeRTOS delays in ticks/ms)
    const uint32_t remaining_us = (uint32_t)(this->step_delay - elapsed);
    const TickType_t ticks = pdMS_TO_TICKS((remaining_us + 999) / 1000); // round up

    if (ticks > 0)
    {
      vTaskDelay(ticks);
    }
    else
    {
      taskYIELD(); // remaining < 1 tick -> at least yield CPU
    }
  }
}

void Stepper::stepAndStop(int number_of_steps) {
  step(number_of_steps);
  pause(); // Disable motor to stop power consumption
}

void Stepper::internalStep(unsigned long now, int &steps_left) {
  // get the timeStamp of when you stepped:
      this->last_step_time = now;
      // increment or decrement the step number,
      // depending on direction:
      if (this->direction == 1)
      {
        this->step_number++;
        if (this->step_number == this->number_of_steps) {
          this->step_number = 0;
        }
      }
      else
      {
        if (this->step_number == 0) {
          this->step_number = this->number_of_steps;
        }
        this->step_number--;
      }
      // decrement the steps left:
      steps_left--;
      // step the motor to step number 0, 1, ..., {3 or 10}
      stepMotor(this->step_number % 4);
}

/*
 * Moves the motor forward or backwards.
 */
void Stepper::stepMotor(int thisStep)
{
  switch (thisStep) {
      case 0:  // 1010
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, HIGH);
        digitalWrite(motor_pin_4, LOW);
      break;
      case 1:  // 0110
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, HIGH);
        digitalWrite(motor_pin_3, HIGH);
        digitalWrite(motor_pin_4, LOW);
      break;
      case 2:  //0101
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, HIGH);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, HIGH);
      break;
      case 3:  //1001
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, HIGH);
      break;
    }
}
