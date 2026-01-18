/**
 * @file main.cpp
 * @brief Main entry point for MedBox Controller
 * 
 * Orchestrates WiFi connection, WebSocket communication, and LED status display.
 * Uses FreeRTOS task for non-blocking LED pattern animation.
 */
#define DEBUG

#include <Arduino.h>
#include <WiFi.h>
#include <gpio.hpp>
#include "network/wifi_helper.hpp"
#include "network/websocket_helper.hpp"
#include "network/communication_helper.hpp"
#include "network/message_parser.hpp"
#include "motor/compartment_set.hpp"
#include "motor/pill_dispenser_stepper.hpp"
#include "motor/rotary_funnel.hpp"

// Global state
bool wifi_connected = false;

// Helper instances
WifiHelper wifiHelper;
WebSocketHelper wsHelper;
CommunicationHelper commHelper;

MessageParser msgParser;

// Motor control instances
PillDispenser* dispenserA = nullptr;
PillDispenser* dispenserB = nullptr;
CompartmentSet* compartmentSet = nullptr;
RotaryFunnel* rotaryFunnel = nullptr;

/**
 * @brief Global LED state pattern (16-bit rotating pattern)
 * 
 * Initial value 0xC0C0 creates an alternating pattern.
 * Modified by various modules to indicate system status.
 */
uint16_t ledState = 0xC0C0;

bool master = false;

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

  // Create LED task on Core 1 for non-blocking status display
  xTaskCreatePinnedToCore(
    ledTask,          // Task function
    "ledTask",        // Task name (for debugging)
    2048,             // Stack size in bytes
    NULL,             // Task parameter (unused)
    1,                // Priority (1 = low)
    NULL,             // Task handle (not needed)
    1                 // Core ID (0 or 1)
  );
  
  master = isMaster();

  // Initialize Compartment Set with Stepper Motors
  // Each compartment uses a 4-wire stepper motor (m1, m2, m3, m4)
  dispenserA = new PillDispenserStepper(
    STEPPER_STEPS_PER_REV,
    COMPARTMENT_A_PIN1,
    COMPARTMENT_A_PIN2,
    COMPARTMENT_A_PIN3,
    COMPARTMENT_A_PIN4
  );
  dispenserB = new PillDispenserStepper(
    STEPPER_STEPS_PER_REV,
    COMPARTMENT_B_PIN1,
    COMPARTMENT_B_PIN2,
    COMPARTMENT_B_PIN3,
    COMPARTMENT_B_PIN4
  );
    // Initialize Rotary Funnel
  rotaryFunnel = new RotaryFunnel(
    STEPPER_STEPS_PER_REV,
    FUNNEL_PIN1,
    FUNNEL_PIN2,
    FUNNEL_PIN3,
    FUNNEL_PIN4
  );

  // Create and initialize compartment set
  compartmentSet = new CompartmentSet(dispenserA, dispenserB, rotaryFunnel);
  compartmentSet->begin();
  
  Serial.println("[Setup] Rotary Funnel initialized");

  // Initialize communication helper (UART, Serial, Parallel pins)
  commHelper.begin(master);
  
  // Attempt WiFi connection (or start BLE config if needed)
  wifi_connected = true;

  // Initialize message parser to handle incoming messages
  msgParser.setTargetBoxMac(WiFi.macAddress());
  
  // Set compartment set in message parser
  msgParser.setCompartmentSet(compartmentSet);

  // Only master device manages WiFi and WebSocket
  if (master) {
    wifi_connected = wifiHelper.connect();

    // Initialize WebSocket if WiFi connection succeeded
    if (wifi_connected) {
      Serial.println("[Setup] WiFi connected, initializing WebSocket...");
      wsHelper.setCommunicationHelper(&commHelper);
      wsHelper.begin();
    } else {
      Serial.println("[Setup] WiFi not connected, BLE configuration active");
    }
    wsHelper.setWebSocketCallback([](const String& data) {
      // Also parse the message directly
      Serial.println("[WebSocket 1] Message received, parsing...");
      msgParser.parseMessage(data);
    });
  } else {
    Serial.println("[Setup] Configured as SLAVE device, skipping WiFi/WebSocket setup");

    // Register UART callback to parse incoming messages
    commHelper.setUartCallback([](const String& data) {
      msgParser.parseMessage(data);
    });

    ledState = 0x0000; // Indicate slave mode with LED pattern
  }
  
  Serial.println("[Setup] Initialization complete");
}

void loop() {
  // Process communication helper (UART data reception)
  commHelper.loop();
  
  if (!wifi_connected) {
    // WiFi not connected - run BLE configuration loop
    wifiHelper.loop();
  } else {
    if (master) {
      // WiFi connected - maintain WebSocket connection
      wsHelper.loop();

      if (wsHelper.shouldEnumerate()) {
        Serial.println("[Loop] WebSocket connected, starting enumeration");
        commHelper.beginUartEnumeration();
      }

      // send periodic heartbeat or status if needed
      //commHelper.sendUart("Heartbeat from MASTER\n");
      
      // Monitor WiFi connection status
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Loop] WiFi connection lost! Restarting...");
        delay(1000);
        ESP.restart();
      }
    }
  }
  
  // Short delay to prevent tight loop
  delay(100);
}
