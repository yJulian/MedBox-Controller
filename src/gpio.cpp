#include "gpio.hpp"
#include <Arduino.h>

void initializeGPIO() {
    // Configure reset pin with pull-up resistor
    // Reading LOW during boot triggers WiFi credential reset
    pinMode(RESET_PIN, INPUT_PULLUP);
    
    // Configure LED pin for output to display status patterns
    pinMode(LED_PIN, OUTPUT);

    // Short delay to stabilize pin states after configuration
    delay(100); 
}