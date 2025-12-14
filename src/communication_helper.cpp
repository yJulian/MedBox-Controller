#include "communication_helper.hpp"
#include "defines.hpp"

// Static instance pointer for ISR
CommunicationHelper* CommunicationHelper::instance = nullptr;

CommunicationHelper::CommunicationHelper() 
    : uart(2), // Use Serial2 (UART2) for inter-MedBox communication
      uartCallback(nullptr),
      serialInputCallback(nullptr),
      uartBuffer(""),
      serialInputChanged(false),
      serialInputState(LOW) {
}

void CommunicationHelper::begin(bool isMaster) {
    // Set static instance for ISR access (only one instance should exist)
    if (instance == nullptr) {
        instance = this;
    } else {
        Serial.println("[CommHelper] Warning: Multiple CommunicationHelper instances detected!");
    }

    if (isMaster) {
        Serial.println("[CommHelper] Configured as MASTER");
        uart.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    } else {
        Serial.println("[CommHelper] Configured as SLAVE");
        uart.begin(115200, SERIAL_8N1, TX_PIN, RX_PIN);
    }
    Serial.println("[CommHelper] UART initialized on Serial2");
    
    attachInterrupt(digitalPinToInterrupt(SERIAL_IN_PIN), serialInputISR, CHANGE);
    Serial.println("[CommHelper] SERIAL_IN_PIN configured with interrupt");

    Serial.println("[CommHelper] All communication interfaces initialized");
}

void CommunicationHelper::loop() {
    // Process serial input interrupt flag (deferred from ISR)
    if (serialInputChanged) {
        serialInputChanged = false;
        if (serialInputCallback != nullptr) {
            serialInputCallback(serialInputState);
        }
    }
    
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
}

void CommunicationHelper::setUartCallback(UartCallback callback) {
    uartCallback = callback;
    Serial.println("[CommHelper] UART callback registered");
}

// ============================================================================
// Serial Pin Communication Methods
// ============================================================================

void CommunicationHelper::pulseSerialOut(uint32_t delayUs = 1000) {
    // Pull SERIAL_OUT_PIN LOW for 1ms pulse
    digitalWrite(SERIAL_OUT_PIN, LOW);
    delayMicroseconds(delayUs); // 1ms pulse
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
    if (instance != nullptr) {
        // Read pin state and set flag for deferred processing in loop()
        // This keeps ISR fast and avoids callback execution in interrupt context
        instance->serialInputState = digitalRead(SERIAL_IN_PIN);
        instance->serialInputChanged = true;
    }
}
