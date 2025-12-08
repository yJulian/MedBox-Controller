#include <Arduino.h>
#include <gpio.hpp>
#include <wifi_helper.hpp>


void setup() {
  // Setup serial communication at 115200 baud rate
  Serial.begin(115200);
  // Initialize GPIOs
  initializeGPIO();

  // Setup WiFi connection
  connectWifi();
}

void loop() {
  // put your main code here, to run repeatedly:
}
