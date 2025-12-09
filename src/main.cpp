#include <Arduino.h>
#include <WiFi.h>
#include <gpio.hpp>
#include <wifi_helper.hpp>
#include <websocket_helper.hpp>

boolean wifi_loop = false;
boolean wifi_connected = false;

WifiHelper wifiHelper;
WebSocketHelper wsHelper;

void setup() {
  // Setup serial communication at 115200 baud rate
  Serial.begin(115200);
  // Initialize GPIOs
  initializeGPIO();
  
  // Setup WiFi connection
  wifi_connected = wifiHelper.connect();
  wifi_loop = !wifi_connected;
  
  // Initialize WebSocket if WiFi is connected
  if (wifi_connected) {
    Serial.println("WiFi connected, initializing WebSocket...");
    wsHelper.begin();
  }
}

void loop() {
  if (wifi_loop) {
    wifiHelper.loop();
  } else {
    // WiFi is connected, run WebSocket loop
    wsHelper.loop();
    
    // Check if WiFi is still connected
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected! Restarting...");
      delay(1000);
      ESP.restart();
    }
  }
  // put your main code here, to run repeatedly:
}
