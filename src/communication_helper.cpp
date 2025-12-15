#include "communication_helper.hpp"
#include "defines.hpp"
#include <WiFi.h>

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
    this->isMaster = isMaster;
    if (isMaster) {
        enumerationUartHandler = std::bind(&CommunicationHelper::enumerationUartMasterHandler, this, std::placeholders::_1);
        uart.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
        Serial.println("[CommHelper] Configured as MASTER");
    } else {
        enumerationUartHandler = std::bind(&CommunicationHelper::enumerationUartSlaveHandler, this, std::placeholders::_1);
        uart.begin(115200, SERIAL_8N1, TX_PIN, RX_PIN);
        Serial.println("[CommHelper] Configured as SLAVE");
        // Slave starts with enumeration handler
        state = ENUMERATION;
    }
    Serial.println("[CommHelper] UART initialized on Serial2");
    
    attachInterrupt(digitalPinToInterrupt(SERIAL_IN_PIN), serialInputISR, CHANGE);
    Serial.println("[CommHelper] SERIAL_IN_PIN configured with interrupt");

    Serial.println("[CommHelper] All communication interfaces initialized");
}

void CommunicationHelper::beginUartEnumeration() {
    state = ENUMERATION;
    uartBuffer = "";

    if (isMaster) {
        setUartCallback(std::bind(&CommunicationHelper::enumerationUartMasterHandler, this, std::placeholders::_1));
    } else {
        Serial.println("[CommHelper] Slave already has enumeration handler set");
    }
    Serial.println("[CommHelper] UART enumeration started");
}


void CommunicationHelper::enumerationUartMasterHandler(const String& data) {
    Serial.printf("[CommHelper] Master received enumeration data: %s\n", data.c_str());
    
    slaves[currentSlaveIdx].idx = currentSlaveIdx;
    slaves[currentSlaveIdx].mac = data;
    Serial.printf("[CommHelper] Registered Slave %u with MAC %s\n", currentSlaveIdx, data.c_str());
    currentSlaveIdx++;
    this->sendUart("ACK");
    lastEnumerationTime = millis();
}

void CommunicationHelper::enumerationUartSlaveHandler(const String& data) {
    if (data == "ENUM_DONE") {
        Serial.println("[CommHelper] Slave received ENUM_DONE, ending enumeration");
        state = NORMAL;
        return;
    }
    if (waitForNextRequest) {
        Serial.printf("[CommHelper] Slave received enumeration request: %s\n", data.c_str());
        waitForNextRequest = false;
        pulseSerialOut();
        return;
    }
    Serial.printf("[CommHelper] Slave received enumeration data: %s\n", data.c_str());
}

void CommunicationHelper::handleSlaveEnumerationRequest() {
    // Implementation for handling slave enumeration request
    String mac = WiFi.macAddress();
    CommunicationHelper::instance->sendUart(mac);
    waitForNextRequest = true;
    Serial.printf("[CommHelper] Slave sent MAC address for enumeration: %s\n", mac.c_str());
}

void CommunicationHelper::loop() {
    // Process serial input interrupt flag (deferred from ISR)
    if (serialInputChanged) {
        serialInputChanged = false;
        // handle Slave Enumeration Request
        if (state == ENUMERATION && !isMaster) {
            handleSlaveEnumerationRequest();
        } else if (serialInputCallback != nullptr) {
            serialInputCallback(serialInputState);
        }
    }
    
    // Check for available UART data
    while (uart.available()) {
        char c = uart.read();
        
        // Buffer incoming data until newline
        if (c == '\n' || c == '\r') {
            // Trigger callback if buffer contains data
            if (uartBuffer.length() > 0) {
                if (state == ENUMERATION)
                    enumerationUartHandler(uartBuffer);
                else if (uartCallback != nullptr)
                    uartCallback(uartBuffer);
            }
            uartBuffer = "";
        } else {
            uartBuffer += c;
        }
    }

    if (state == ENUMERATION && millis() - lastEnumerationTime < 1000) {
        // End enumeration
        state = NORMAL;
        Serial.println("[CommHelper] UART enumeration completed");
        this->sendUart("ENUM_DONE");
        // todo notify upper layer
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

void CommunicationHelper::pulseSerialOut(uint32_t delayUs) {
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
