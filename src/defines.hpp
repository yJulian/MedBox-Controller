#pragma once
#include <cstdint>
#define RESET_PIN  15  // Define the reset pin
#define LED_PIN    2   // Define the LED pin

// WebSocket Configuration
#define WS_HOST "192.168.0.132"  // WebSocket server host/IP
#define WS_PORT 8080              // WebSocket server port
#define WS_PATH "ws"             // WebSocket endpoint path
#define WS_RECONNECT_INTERVAL 5000  // Reconnection interval in milliseconds
#define WS_PING_INTERVAL 30000      // Ping interval for keep-alive in milliseconds

extern uint16_t ledState;