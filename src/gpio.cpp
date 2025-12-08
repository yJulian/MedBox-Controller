#ifndef GPIO_HPP
#define GPIO_HPP

#include <gpio.hpp>
#include <Arduino.h>

void initializeGPIO() {
    // Initialize GPIO pins here
    pinMode(RESET_PIN, INPUT_PULLUP);

    // Short delay to stabilize the pin state
    delay(100); 
}

#endif // GPIO_HPP