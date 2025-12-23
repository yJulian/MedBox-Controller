#include "motor.hpp"
#include <Stepper.h>

#define STEPS 200 
Stepper stepper(STEPS, 8, 9, 10, 11);

Motor::Motor() {
    // Constructor implementation (if needed)
}

void Motor::initialize() {
    stepper.setSpeed(30);
}

void Motor::setSpeed(double speed) {
    stepper.setSpeed(speed);
}

void Motor::step(int steps) {
    stepper.step(steps);
}

void Motor::stop() {
    // Stopping a stepper motor typically involves simply not sending further step commands.
    // You might want to implement a method to disable the motor if your hardware supports it.
}
