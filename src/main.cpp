#include <Arduino.h>
#include <WiFi.h>
#include <gpio.hpp>
#include <wifi_helper.hpp>
#include <websocket_helper.hpp>

bool wifi_connected = false;

WifiHelper wifiHelper;
WebSocketHelper wsHelper;
uint16_t ledState = 0xC0C0; // Initial LED state

void ledTask(void *param) {
  for (;;) {
    ledState = (ledState << 1) | (ledState >> 15); // Rotate left
      for (int i = 0; i < 16; i++) {
        digitalWrite(LED_PIN, (ledState & (1 << i)) ? HIGH : LOW);
      }
      vTaskDelay(pdMS_TO_TICKS(100));  
  }
}


void setup() {
  // Setup serial communication at 115200 baud rate
  Serial.begin(115200);
  // Initialize GPIOs
  initializeGPIO();
  
  // Setup WiFi connection
  wifi_connected = wifiHelper.connect();
  
  // Initialize LED state
  xTaskCreatePinnedToCore(
        ledTask,          // Funktion
        "ledTask",        // Name
        4096,            // Stack
        NULL,            // Parameter
        1,               // Priority
        NULL,            // Handle
        1                // Core (0 oder 1)
    );

  // Initialize WebSocket if WiFi is connected
  if (wifi_connected) {
    Serial.println("WiFi connected, initializing WebSocket...");
    wsHelper.begin();
  }
}


void loop() {
  if (!wifi_connected) {
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
  delay(100);
}
