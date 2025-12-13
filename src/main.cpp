/**
 * @file main.cpp
 * @brief Main entry point for MedBox Controller
 * 
 * Orchestrates WiFi connection, WebSocket communication, and LED status display.
 * Uses FreeRTOS task for non-blocking LED pattern animation.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <gpio.hpp>
#include <wifi_helper.hpp>
#include <websocket_helper.hpp>

// Global state
bool wifi_connected = false;

// Helper instances
WifiHelper wifiHelper;
WebSocketHelper wsHelper;

/**
 * @brief Global LED state pattern (16-bit rotating pattern)
 * 
 * Initial value 0xC0C0 creates an alternating pattern.
 * Modified by various modules to indicate system status.
 */
uint16_t ledState = 0xC0C0;

/**
 * @brief FreeRTOS task for LED status display
 * 
 * Runs on Core 1 to provide non-blocking LED animation.
 * Rotates the 16-bit ledState pattern left and displays each bit
 * sequentially on the LED pin with 100ms delay.
 * 
 * @param param Unused task parameter
 */
void ledTask(void *param) {
  for (;;) {
    // Rotate pattern left by 1 bit (circular shift)
    ledState = (ledState << 1) | (ledState >> 15);
    
    // Display each bit of the pattern sequentially
    for (int i = 0; i < 16; i++) {
      digitalWrite(LED_PIN, (ledState & (1 << i)) ? HIGH : LOW);
    }
    
    // Wait 100ms before next update
    vTaskDelay(pdMS_TO_TICKS(100));  
  }
}

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  Serial.println("\n[Setup] MedBox Controller starting...");
  
  // Configure GPIO pins (LED and Reset button)
  initializeGPIO();
  
  // Attempt WiFi connection (or start BLE config if needed)
  wifi_connected = wifiHelper.connect();
  
  // Create LED task on Core 1 for non-blocking status display
  xTaskCreatePinnedToCore(
        ledTask,          // Task function
        "ledTask",        // Task name (for debugging)
        4096,             // Stack size in bytes
        NULL,             // Task parameter (unused)
        1,                // Priority (1 = low)
        NULL,             // Task handle (not needed)
        1                 // Core ID (0 or 1, using 1)
    );

  // Initialize WebSocket if WiFi connection succeeded
  if (wifi_connected) {
    Serial.println("[Setup] WiFi connected, initializing WebSocket...");
    wsHelper.begin();
  } else {
    Serial.println("[Setup] WiFi not connected, BLE configuration active");
  }
  
  Serial.println("[Setup] Initialization complete");
}

void loop() {
  if (!wifi_connected) {
    // WiFi not connected - run BLE configuration loop
    wifiHelper.loop();
  } else {
    // WiFi connected - maintain WebSocket connection
    wsHelper.loop();
    
    // Monitor WiFi connection status
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[Loop] WiFi connection lost! Restarting...");
      delay(1000);
      ESP.restart();
    }
  }
  
  // Short delay to prevent tight loop
  delay(100);
}
