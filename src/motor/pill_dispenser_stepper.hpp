#pragma once

#include "pill_dispenser.hpp"
#include "stepper/Stepper.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class PillDispenserStepper: public PillDispenser {
public:
    PillDispenserStepper(int number_of_stepsm, int m1, int m2, int m3, int m4);
    
    void dispensePillCompartmentA();
    void dispensePillCompartmentB();

    void begin();

private:
    int number_of_steps;
    int m1;
    int m2;
    int m3;
    int m4;
    SemaphoreHandle_t mux;

    Stepper stepperMotor;
};