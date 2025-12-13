#pragma once
#include <cstdint>

/**
 * @file defines.hpp
 * @brief Central configuration file for MedBox Controller
 * 
 * Contains all hardware pin definitions, WebSocket configuration,
 * and global state variables.
 */

// ============================================================================
// GPIO Pin Definitions
// ============================================================================

/**
 * @brief Reset button pin (active LOW with internal pull-up)
 * 
 * When held LOW during boot, triggers WiFi credential reset.
 * Device will start BLE GATT server for reconfiguration.
 */
#define RESET_PIN  15

/**
 * @brief Status LED pin
 * 
 * Used to display various status patterns:
 * - 0x0303: WiFi connected successfully
 * - 0xFF00: WiFi connecting / attempting connection
 * - 0xCCCC: BLE GATT server advertising
 * - 0xAAAA: BLE client connected
 * - 0x0000: WebSocket connected
 * - 0xC0C0: Default pattern (rotating bits)
 */
#define LED_PIN    2

// ============================================================================
// WebSocket Configuration
// ============================================================================

/**
 * @brief WebSocket server hostname or IP address
 */
#define WS_HOST "192.168.0.132"

/**
 * @brief WebSocket server port
 */
#define WS_PORT 8080

/**
 * @brief WebSocket endpoint path (without leading slash)
 */
#define WS_PATH "ws"

/**
 * @brief Auto-reconnection interval in milliseconds
 * 
 * If connection is lost, the client will attempt to reconnect
 * after this interval.
 */
#define WS_RECONNECT_INTERVAL 5000

/**
 * @brief Heartbeat ping interval in milliseconds
 * 
 * Keeps the WebSocket connection alive by sending periodic
 * ping messages to the server.
 */
#define WS_PING_INTERVAL 30000

// ============================================================================
// Global State Variables
// ============================================================================

/**
 * @brief Current LED display pattern
 * 
 * 16-bit value where each bit represents LED state in rotation.
 * Modified by various modules to indicate system status.
 */
extern uint16_t ledState;