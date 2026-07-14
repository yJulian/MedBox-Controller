#pragma once
#include <cstdint>

#define MAX_SLAVES 10



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
 * @brief Special offline mode pin (active LOW with internal pull-up)
 * 
 * When held LOW during boot, triggers offline BLE control mode.
 * Device will host a BLE GATT server for motor control instead of connecting to WiFi.
 */
#define SPECIAL_MODE_PIN  4

#define BLE_CONTROL_SERVICE_UUID "0000ff01-0000-1000-8000-00805f9b34fb"
#define BLE_CONTROL_CHAR_UUID    "0000ff02-0000-1000-8000-00805f9b34fb"

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

/**
 * @brief Serial communication pins
 * 
 * Configured for serial arbitration between multiple peripherals.
 * Output from one peripheral is routed to the input the next in the chain.
 * 
 * First peripheral has no input (RX) pin connected.
 */
#define SERIAL_IN_PIN 19
#define SERIAL_OUT_PIN 13
/**
 * @brief Parallel communication pin
 * 
 * Used for parallel arbitration between peripherals.
 * Output from one peripheral is routed to the input of all other peripherals.
 * Used as wired-and for broadcasting data.
 */
#define PARALLEL_PIN 21

/**
 * @brief UART pins for communication between master and slave devices
 * 
 * Master device TX connects to Slave device RX and vice versa. 
 */
#define TX_PIN 33
#define RX_PIN 16

// ============================================================================
// Motor Control Pins
// ============================================================================

/**
 * PIN Ordering Help 
 * Pin on Motor Driver -> #Pin
 * -------------------------
 * IN1            -> Pin1
 * IN2            -> Pin3
 * IN3            -> Pin2
 * IN4            -> Pin4
 */

#define DEBUG

/**
 * @brief Compartment A Stepper Motor Pins (4-wire)
 * 
 * Four pins for controlling the stepper motor coils (m1, m2, m3, m4)
 * Used with the Stepper library for precise step control
 */
#define COMPARTMENT_A_PIN1 32 // 22
#define COMPARTMENT_A_PIN3 26 // 18
#define COMPARTMENT_A_PIN2 25 // 23
#define COMPARTMENT_A_PIN4 27 // 5
#define COMPARTMENT_A_ENCODER 34

/**
 * @brief Compartment B Stepper Motor Pins (4-wire)
 * 
 * Four pins for controlling the stepper motor coils (m1, m2, m3, m4)
 * Used with the Stepper library for precise step control
 */
#define COMPARTMENT_B_PIN1 14
#define COMPARTMENT_B_PIN3 17
#define COMPARTMENT_B_PIN2 16
#define COMPARTMENT_B_PIN4 19
#define COMPARTMENT_B_ENCODER 35

/**
 * @brief Rotary Funnel Stepper Motor Pins (4-wire)
 * 
 * Four pins for controlling the rotary funnel stepper motor coils (m1, m2, m3, m4)
 * Used to rotate the funnel between different positions (0°, 90°, 180°, 270°)
 */
#define FUNNEL_PIN1 22 // 32 
#define FUNNEL_PIN3 18 // 26 
#define FUNNEL_PIN2 23 // 25 
#define FUNNEL_PIN4 5 // 27 

/**
 * @brief Steps per revolution for stepper motors
 * 
 * This value depends on the specific stepper motor model.
 * Standard NEMA 17: 200 steps/rev, NEMA 23: 200 steps/rev
 */
#define STEPPER_STEPS_PER_REV 2048

// ============================================================================
// WebSocket Configuration
// ============================================================================

/**
 * @brief WebSocket server hostname or IP address
 */
#define WS_HOST "23.88.97.42"

/**
 * @brief WebSocket server port
 */
#define WS_PORT 8081

/**
 * @brief WebSocket endpoint path
 */
#define WS_PATH "/device/"

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