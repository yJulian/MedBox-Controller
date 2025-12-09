#include <Arduino.h>
#include <gpio.hpp>
#include <wifi_helper.hpp>

boolean wifi_loop = false;

WifiHelper wifiHelper;

void setup() {
  // Setup serial communication at 115200 baud rate
  Serial.begin(115200);
  // Initialize GPIOs
  initializeGPIO();

  
  
  
  // Setup WiFi connection
  wifi_loop = true;
  wifiHelper.connect();
  wifi_loop = false;
}

void loop() {
  if (wifi_loop) {
    wifiHelper.loop();
  }
  // put your main code here, to run repeatedly:
}
