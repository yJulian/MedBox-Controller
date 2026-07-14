#include "pill_dispenser_encoder.hpp"
#include <Arduino.h>
#include "defines.hpp"

#define STEP_DELAY_MS 1200
#define DEGRESS_PER_PILL 90

#define SPEED_RPM 8

#define FAST_CALIBRATE_FACTOR 4

// Analog reading above this threshold means the light barrier is triggered (home position reached)
#define CONTROL_SWITCH_THRESHOLD 3000

PillDispenserEncoder::PillDispenserEncoder(int number_of_steps, int m1, int m2, int m3, int m4, uint8_t control_switch)
    : PillDispenserStepper(number_of_steps, m1, m2, m3, m4) {
        this->control_switch = control_switch;
        pinMode(control_switch, INPUT_PULLUP);
}

void PillDispenserEncoder::begin() {
    stepperMotor.begin();
    stepperMotor.setSpeed(SPEED_RPM);
    mux = xSemaphoreCreateMutex();
}

// Steps back towards the home position one step at a time, stopping as soon as
// the light barrier triggers or DEGRESS_PER_PILL*2 has been travelled, whichever
// comes first.
void PillDispenserEncoder::returnToHome(int direction) {
    uint16_t max_steps = DEGRESS_PER_PILL * 2 * STEPPER_STEPS_PER_REV / 360;
    for (uint16_t i = 0; i < max_steps; i++) {
        int switch_analog = analogRead(control_switch);
        Serial.printf("[PillDispenserEncoder] returnToHome step %d/%d, control_switch_analog=%d\n", i, max_steps, switch_analog);
        if (switch_analog > CONTROL_SWITCH_THRESHOLD) {
            break;
        }
        stepperMotor.step(direction);
    }
    stepperMotor.pause();
}

// Default to motor beeing in position 0 at startup
// No way to check actual position without limit switches
void PillDispenserEncoder::dispensePillCompartmentA() {
    xSemaphoreTake(mux, portMAX_DELAY);
    stepperMotor.step(DEGRESS_PER_PILL * STEPPER_STEPS_PER_REV / 360);
    vTaskDelay(STEP_DELAY_MS / portTICK_PERIOD_MS);
    returnToHome(-1);
    xSemaphoreGive(mux);
}

void PillDispenserEncoder::dispensePillCompartmentB() {
    xSemaphoreTake(mux, portMAX_DELAY);
    stepperMotor.step(-DEGRESS_PER_PILL * STEPPER_STEPS_PER_REV / 360);
    vTaskDelay(STEP_DELAY_MS / portTICK_PERIOD_MS);
    returnToHome(1);
    xSemaphoreGive(mux);
}

bool PillDispenserEncoder::calibrate() {
    bool calibrated = false;
    for(uint8_t current_factor = 1; current_factor < FAST_CALIBRATE_FACTOR; current_factor++) {
        uint16_t steps = -DEGRESS_PER_PILL * STEPPER_STEPS_PER_REV * current_factor / FAST_CALIBRATE_FACTOR / 360;
        stepperMotor.step(steps);
        for (uint16_t i = 0; i < steps; i++) {
            if (analogRead(control_switch) > CONTROL_SWITCH_THRESHOLD) {
                calibrated = true;
                break;
            }
            vTaskDelay(1 / portTICK_PERIOD_MS);
            stepperMotor.step(-1);
        }
        if (calibrated) {
            break;
        }
    }
    return calibrated;
}
