#include <Arduino.h>
#include <WiFi.h>
#include <gpio.hpp>
#include <wifi_helper.hpp>
#include <websocket_helper.hpp>

bool wifi_connected = false;

WifiHelper wifiHelper;
WebSocketHelper wsHelper;

bool p2LedState = false;

void setup() {
  // Setup serial communication at 115200 baud rate
  Serial.begin(115200);
  // Initialize GPIOs
  initializeGPIO();
  
  // Setup WiFi connection
  wifi_connected = wifiHelper.connect();
  
  // Initialize WebSocket if WiFi is connected
  if (wifi_connected) {
    Serial.println("WiFi connected, initializing WebSocket...");
    wsHelper.begin();
  }
}

void loop() {
  if (!wifi_connected) {
    wifiHelper.loop();

    digitalWrite(LED_PIN, (p2LedState = !p2LedState));
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
}
