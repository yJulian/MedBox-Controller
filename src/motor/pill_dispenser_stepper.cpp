#include "pill_dispenser_stepper.hpp"
#include <Arduino.h>

#define STEP_DELAY_MS 800
#define STEPS_PER_PILL 200

#define SPEED_RPM 16

PillDispenserStepper::PillDispenserStepper(int number_of_steps, int m1, int m2, int m3, int m4)
    : stepperMotor(number_of_steps, m1, m2, m3, m4) {
    this->number_of_steps = number_of_steps;
    this->m1 = m1;
    this->m2 = m2;
    this->m3 = m3;
    this->m4 = m4;
}

void PillDispenserStepper::begin() {
    stepperMotor.setSpeed(SPEED_RPM);
}

// Default to motor beeing in position 0 at startup
// No way to check actual position without limit switches
void PillDispenserStepper::dispensePillCompartmentA() {
    // Example: Dispense from compartment A
    stepperMotor.step(STEPS_PER_PILL);
    vTaskDelay(STEP_DELAY_MS / portTICK_PERIOD_MS);
    stepperMotor.step(-STEPS_PER_PILL);
}
void PillDispenserStepper::dispensePillCompartmentB() {
    // Example: Dispense from compartment B
    stepperMotor.step(-STEPS_PER_PILL);
    vTaskDelay(STEP_DELAY_MS / portTICK_PERIOD_MS);
    stepperMotor.step(STEPS_PER_PILL);
}

