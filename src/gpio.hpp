#ifndef GPIO_HPP
#define GPIO_HPP

#include <defines.hpp>

/**
 * @brief Initialize GPIO pins for the MedBox controller
 * 
 * Configures:
 * - RESET_PIN as INPUT_PULLUP for WiFi configuration reset
 * - LED_PIN as OUTPUT for status indication
 */
void initializeGPIO();

/**
 * @brief Check if this device is configured as Master
 * 
 * @return true if Master, false if Slave
 */
bool isMaster();

#endif // GPIO_HPP