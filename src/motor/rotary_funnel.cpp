#include "rotary_funnel.hpp"
#include <Arduino.h>

#define SPEED_RPM 10

RotaryFunnel::RotaryFunnel(int number_of_steps, int m1, int m2, int m3, int m4)
    : number_of_steps(number_of_steps), m1(m1), m2(m2), m3(m3), m4(m4),
      stepperMotor(number_of_steps, m1, m2, m3, m4) {
    currentPosition = POSITION_0;
}

void RotaryFunnel::begin() {
    prefs.begin("funnel", false);
    prefs.getInt("position", 0);
    
    stepperMotor.setSpeed(SPEED_RPM);
    stepperMotor.begin();
    mux = xSemaphoreCreateMutex();
}

// Write a method to get the FunnelPosition from a uint16_t
RotaryFunnel::FunnelPosition RotaryFunnel::getPositionFromUint16(uint16_t position) {
    switch (position) {
        case 0:
            return FunnelPosition::POSITION_0;
        case 1:
            return FunnelPosition::POSITION_90;
        case 2:
            return FunnelPosition::POSITION_180;
        case 3:
            return FunnelPosition::POSITION_270;
        default:
            // Default to POSITION_0 if unknown
            return FunnelPosition::POSITION_0;
    }
}

int RotaryFunnel::getDegrees(FunnelPosition position) {
    switch (position) {
        case POSITION_0:
            return 0;
        case POSITION_90:
            return 90;
        case POSITION_180:
            return 180;
        case POSITION_270:
            return 270;
        default:
            return 0;
    }
}

void RotaryFunnel::rotateToPosition(FunnelPosition position) {
    // Calculate the shortest path from current to target position
    // Each position represents 90 degrees
    int currentDegrees = prefs.getInt("position", 0);
    int targetDegrees = getDegrees(position);

    printf("[RotaryFunnel] Current position: %d°, Target position: %d°\n", currentDegrees, targetDegrees);
    
    // Calculate angular difference (-270 to +270)
    int difference = targetDegrees - currentDegrees;

    // Normalize to shortest path (-180 to +180)
    if (difference > 180) {
        difference -= 360;
    } else if (difference < -180) {
        difference += 360;
    }

    // print the difference
    Serial.printf("[RotaryFunnel] Difference: %d°\n", difference);
    
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
        prefs.putInt("position", targetDegrees);
        currentPosition = position;
        xSemaphoreGive(mux);
    }
}
