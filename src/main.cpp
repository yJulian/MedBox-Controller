#include <Arduino.h>
#include <gpio.hpp>
#include <wifi_helper.hpp>


void setup() {
  // Setup serial communication at 115200 baud rate
  Serial.begin(115200);
  // Initialize GPIOs
  initializeGPIO();

  WifiHelper wifiHelper;

  // Setup WiFi connection
  wifiHelper.connect();
}

void loop() {
  // put your main code here, to run repeatedly:
}
