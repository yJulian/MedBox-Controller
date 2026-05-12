#include "pill_dispenser_encoder.hpp"
#include <Arduino.h>
#include "defines.hpp"

#define STEP_DELAY_MS 1200
#define DEGRESS_PER_PILL 90

#define SPEED_RPM 8

#define FAST_CALIBRATE_FACTOR 4

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

// Default to motor beeing in position 0 at startup
// No way to check actual position without limit switches
void PillDispenserEncoder::dispensePillCompartmentA() {
    xSemaphoreTake(mux, portMAX_DELAY);
    stepperMotor.step(DEGRESS_PER_PILL * STEPPER_STEPS_PER_REV / 360);
    vTaskDelay(STEP_DELAY_MS / portTICK_PERIOD_MS);
    stepperMotor.stepAndStop(-DEGRESS_PER_PILL * STEPPER_STEPS_PER_REV / 360);
    xSemaphoreGive(mux);
}

void PillDispenserEncoder::dispensePillCompartmentB() {
    xSemaphoreTake(mux, portMAX_DELAY);
    stepperMotor.step(-DEGRESS_PER_PILL * STEPPER_STEPS_PER_REV / 360);
    vTaskDelay(STEP_DELAY_MS / portTICK_PERIOD_MS);
    stepperMotor.stepAndStop(DEGRESS_PER_PILL * STEPPER_STEPS_PER_REV / 360);
    xSemaphoreGive(mux);
}

bool PillDispenserEncoder::calibrate() {
    bool calibrated = false;
    for(uint8_t current_factor = 1; current_factor < FAST_CALIBRATE_FACTOR; current_factor++) {
        uint16_t steps = -DEGRESS_PER_PILL * STEPPER_STEPS_PER_REV * current_factor / FAST_CALIBRATE_FACTOR / 360;
        stepperMotor.step(steps);
        for (uint16_t i = 0; i < steps; i++) {
            if (digitalRead(control_switch) == LOW) {
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
