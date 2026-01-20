#include "rotary_funnel.hpp"
#include <Arduino.h>

#define SPEED_RPM 10

RotaryFunnel::RotaryFunnel(int number_of_steps, int m1, int m2, int m3, int m4)
    : number_of_steps(number_of_steps), m1(m1), m2(m2), m3(m3), m4(m4),
      stepperMotor(number_of_steps, m1, m2, m3, m4) {
    currentPosition = POSITION_0;
}

void RotaryFunnel::begin() {
    stepperMotor.setSpeed(SPEED_RPM);
    stepperMotor.begin();
    mux = xSemaphoreCreateMutex();
}

void RotaryFunnel::rotateToPosition(FunnelPosition position) {
    // Calculate the shortest path from current to target position
    // Each position represents 90 degrees
    int currentDegrees = static_cast<int>(currentPosition) * 90;
    int targetDegrees = static_cast<int>(position) * 90;
    
    // Calculate angular difference (-270 to +270)
    int difference = targetDegrees - currentDegrees;
    
    // Normalize to shortest path (-180 to +180)
    if (difference > 180) {
        difference -= 360;
    } else if (difference < -180) {
        difference += 360;
    }
    
    // Calculate steps needed (360 degrees = number_of_steps)
    int stepsToMove = (difference * number_of_steps) / 360;
    
    #ifdef DEBUG
    Serial.printf("[RotaryFunnel] Moving from %d° to %d°, rotating %d° (%d steps)\n", 
                  currentDegrees, targetDegrees, difference, stepsToMove);
    #endif
    
    // Rotate to target position
    if (stepsToMove != 0) {
        xSemaphoreTake(mux, portMAX_DELAY);
        stepperMotor.stepAndStop(stepsToMove);
        currentPosition = position;
        xSemaphoreGive(mux);
    }
}
