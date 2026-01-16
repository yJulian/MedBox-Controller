#pragma once

#include "stepper/Stepper.h"

class RotaryFunnel {
public:
    RotaryFunnel(int number_of_steps, int m1, int m2, int m3, int m4);

    enum FunnelPosition {
        POSITION_0,
        POSITION_90,
        POSITION_180,
        POSITION_270
    };  
    
    void rotateToPosition(FunnelPosition position);

    void begin();

private:
    int number_of_steps;
    int m1;
    int m2;
    int m3;
    int m4;

    FunnelPosition currentPosition = POSITION_0;

    Stepper stepperMotor;
};