#include "communication_helper.hpp"
#include "defines.hpp"

// Static instance pointer for ISR
CommunicationHelper* CommunicationHelper::instance = nullptr;

CommunicationHelper::CommunicationHelper() 
    : uart(2), // Use Serial2 (UART2) for inter-MedBox communication
      uartCallback(nullptr),
      serialInputCallback(nullptr),
      uartBuffer("") {
    // Set static instance for ISR access
    instance = this;
}

void CommunicationHelper::begin() {
    // ========================================================================
    // Initialize UART on Serial2 (TX_PIN 17, RX_PIN 16)
    // ========================================================================
    uart.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    Serial.println("[CommHelper] UART initialized on Serial2 (TX:17, RX:16)");
    
    // ========================================================================
    // Initialize Serial pins for chain communication
    // ========================================================================
    // SERIAL_IN_PIN: Configure as input and attach interrupt
    pinMode(SERIAL_IN_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(SERIAL_IN_PIN), serialInputISR, CHANGE);
    Serial.println("[CommHelper] SERIAL_IN_PIN configured with interrupt");
    
    // SERIAL_OUT_PIN: Configure as output, initially HIGH
    pinMode(SERIAL_OUT_PIN, OUTPUT);
    digitalWrite(SERIAL_OUT_PIN, HIGH);
    Serial.println("[CommHelper] SERIAL_OUT_PIN configured as output (HIGH)");
    
    // ========================================================================
    // Initialize Parallel pin for wired-AND communication
    // ========================================================================
    // PARALLEL_PIN: Configure as INPUT (high-impedance Z-state)
    // This allows wired-AND operation where any device can pull LOW
    pinMode(PARALLEL_PIN, INPUT);
    Serial.println("[CommHelper] PARALLEL_PIN configured as input (Z-state for wired-AND)");
    
    Serial.println("[CommHelper] All communication interfaces initialized");
}

void CommunicationHelper::loop() {
    // Check for available UART data
    while (uart.available()) {
        char c = uart.read();
        
        // Buffer incoming data until newline
        if (c == '\n' || c == '\r') {
            // Trigger callback if buffer contains data
            if (uartBuffer.length() > 0 && uartCallback != nullptr) {
                uartCallback(uartBuffer);
            }
            uartBuffer = "";
        } else {
            uartBuffer += c;
        }
    }
}

// ============================================================================
// UART Communication Methods
// ============================================================================

void CommunicationHelper::sendUart(const String& message) {
    uart.println(message);
    Serial.print("[CommHelper] UART sent: ");
    Serial.println(message);
}

void CommunicationHelper::setUartCallback(UartCallback callback) {
    uartCallback = callback;
    Serial.println("[CommHelper] UART callback registered");
}

// ============================================================================
// Serial Pin Communication Methods
// ============================================================================

void CommunicationHelper::pulseSerialOut() {
    // Pull SERIAL_OUT_PIN LOW for 1ms pulse
    digitalWrite(SERIAL_OUT_PIN, LOW);
    delayMicroseconds(1000); // 1ms pulse
    digitalWrite(SERIAL_OUT_PIN, HIGH);
    
    Serial.println("[CommHelper] SERIAL_OUT_PIN pulse sent");
}

void CommunicationHelper::setSerialInputCallback(SerialInputCallback callback) {
    serialInputCallback = callback;
    Serial.println("[CommHelper] SERIAL_IN_PIN callback registered");
}

// ============================================================================
// Parallel Pin Communication Methods (Wired-AND)
// ============================================================================

void CommunicationHelper::pulseParallelPin() {
    // Step 1: Temporarily configure as OUTPUT to drive the line
    pinMode(PARALLEL_PIN, OUTPUT);
    
    // Step 2: Pull to GND (LOW) to send pulse in wired-AND configuration
    digitalWrite(PARALLEL_PIN, LOW);
    delayMicroseconds(1000); // 1ms pulse
    
    // Step 3: Release back to INPUT (high-impedance Z-state)
    // This allows other devices to control the line and enables wired-AND
    pinMode(PARALLEL_PIN, INPUT);
    
    Serial.println("[CommHelper] PARALLEL_PIN pulse sent (wired-AND)");
}

// ============================================================================
// ISR Handlers
// ============================================================================

void IRAM_ATTR CommunicationHelper::serialInputISR() {
    if (instance != nullptr && instance->serialInputCallback != nullptr) {
        // Read current state of SERIAL_IN_PIN
        int state = digitalRead(SERIAL_IN_PIN);
        
        // Trigger callback with current state
        instance->serialInputCallback(state);
    }
}
