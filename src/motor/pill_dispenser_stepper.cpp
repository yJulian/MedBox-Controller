#include "pill_dispenser_stepper.hpp"
#include <Arduino.h>

#define STEP_DELAY_MS 800
#define STEPS_PER_PILL 200

PillDispenserStepper::PillDispenserStepper(int stepPin, int dirPin, int enablePin)
    : stepPin(stepPin), dirPin(dirPin), enablePin(enablePin) {
}

void PillDispenserStepper::begin() {
    // todo replace stepper with library
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, LOW); // Enable the stepper motor driver
}

// Default to motor beeing in position 0 at startup
// No way to check actual position without limit switches
void PillDispenserStepper::dispensePillCompartmentA() {
    // Example: Dispense from compartment A
    stepMotor(STEPS_PER_PILL, true); // 200 steps forward
    vTaskDelay(STEP_DELAY_MS / portTICK_PERIOD_MS);
    stepMotor(STEPS_PER_PILL, false); // 200 steps forward
}
void PillDispenserStepper::dispensePillCompartmentB() {
    // Example: Dispense from compartment B
    stepMotor(STEPS_PER_PILL, false); // 200 steps forward
    vTaskDelay(STEP_DELAY_MS / portTICK_PERIOD_MS);
    stepMotor(STEPS_PER_PILL, true); // 200 steps forward
}

void PillDispenserStepper::stepMotor(int steps, bool direction) {
    digitalWrite(dirPin, direction ? HIGH : LOW);
    for (int i = 0; i < steps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(800); // Adjust delay for speed control
        digitalWrite(stepPin, LOW);
        delayMicroseconds(800);
    }
}

