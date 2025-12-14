#include "gpio.hpp"
#include <Arduino.h>

void initializeGPIO() {
    // Configure reset pin with pull-up resistor
    // Reading LOW during boot triggers WiFi credential reset
    pinMode(RESET_PIN, INPUT_PULLUP);
    
    // Configure LED pin for output to display status patterns
    pinMode(LED_PIN, OUTPUT);

    pinMode(SERIAL_IN_PIN, INPUT_PULLDOWN);
    pinMode(SERIAL_OUT_PIN, OUTPUT);
    // pinMode(PARALLEL_PIN, OUTPUT);
    pinMode(PARALLEL_PIN, INPUT); // Drive as open-collector by default
    
    // Initialize output pins to known states
    digitalWrite(SERIAL_OUT_PIN, HIGH);

    // Short delay to stabilize pin states after configuration
    delay(100); 
}

bool isMaster() {
    // Determine Master/Slave role based on SERIAL_IN_PIN state at boot
    return digitalRead(SERIAL_IN_PIN) == LOW;
}