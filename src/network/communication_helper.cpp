#include "communication_helper.hpp"
#include "websocket_helper.hpp"
#include "defines.hpp"
#include <WiFi.h>
#include <driver/uart.h>
#include <ArduinoJson.h>

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
        uart.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
        // Use pull-down on RX pin (idle state is now LOW with inversion)
        Serial.println("[CommHelper] Configured as MASTER with inverted UART and RX pull-down");
    } else {
        enumerationUartHandler = std::bind(&CommunicationHelper::enumerationUartSlaveHandler, this, std::placeholders::_1);
        uart.begin(9600, SERIAL_8N1, TX_PIN, RX_PIN);
        // Use pull-down on RX pin (TX_PIN for slave due to swapped pins)
        Serial.println("[CommHelper] Configured as SLAVE with inverted UART and RX pull-down");
        // Slave starts with enumeration handler
    }
    Serial.println("[CommHelper] UART initialized on Serial2");
    uart.flush();

    attachInterrupt(digitalPinToInterrupt(SERIAL_IN_PIN), serialInputISR, CHANGE);
    Serial.println("[CommHelper] SERIAL_IN_PIN configured with interrupt");

    sendUart("\n"); // Send newline to reset any partial commands

    Serial.println("[CommHelper] All communication interfaces initialized");
}

void CommunicationHelper::beginUartEnumeration() {
    state = ENUMERATION;
    lastEnumerationTime = millis();
    this->sendUart("ENUM_START");
    delay(1000); // Give slaves time to prepare
    this->currentSlaveIdx = 0;
    pulseSerialOut();
    Serial.println("[CommHelper] UART enumeration started");
}


void CommunicationHelper::enumerationUartMasterHandler(const String& data) {
    Serial.printf("[CommHelper] Master received enumeration data: %s\n", data.c_str());
    
    slaves[this->currentSlaveIdx].idx = this->currentSlaveIdx;
    slaves[this->currentSlaveIdx].mac = data;
    Serial.printf("[CommHelper] Registered Slave %u with MAC %s\n", this->currentSlaveIdx, data.c_str());
    this->currentSlaveIdx++;
    delay(300); // Short delay before sending ACK
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
        Serial.printf("[CommHelper] ACK Received: %s\n", data.c_str());
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
        
        Serial.print(c); // Echo received char for debugging
        
        // Buffer incoming data until newline
        if (c == '\n' || c == '\r') {
            // Trigger callback if buffer contains data
            if (uartBuffer.length() > 0) {
                if (!isMaster && uartBuffer == "ENUM_START") {
                    state = ENUMERATION;
                    Serial.println("[CommHelper] Slave entering ENUMERATION state");
                } else if (state == ENUMERATION)
                    enumerationUartHandler(uartBuffer);
                else if (uartCallback != nullptr)
                    uartCallback(uartBuffer);
            }
            uartBuffer = "";
        } else {
            uartBuffer += c;
        }
    }

    if (this->isMaster && state == ENUMERATION && millis() - lastEnumerationTime > 5000) {
        // End enumeration
        Serial.println("[CommHelper] UART enumeration completed");
        for(SlaveInfo slave : this->slaves) {
            Serial.printf("[CommHelper] Slave %u - MAC: %s\n", slave.idx, slave.mac.c_str());
        }
        this->sendUart("ENUM_DONE");
        state = NORMAL;
        
        // Send enumeration results via WebSocket if connected
        if (webSocketHelper != nullptr && webSocketHelper->isConnected()) {
            JsonDocument doc;
            JsonArray slavesArray = doc["slaves"].to<JsonArray>();
            
            for (uint8_t i = 0; i < currentSlaveIdx; i++) {
                JsonObject slaveObj = slavesArray.add<JsonObject>();
                slaveObj["idx"] = slaves[i].idx;
                slaveObj["mac"] = slaves[i].mac;
            }
            
            String message;
            serializeJson(doc, message);
            webSocketHelper->sendMessage(message);
            Serial.printf("[CommHelper] Sent enumeration results via WebSocket: %s\n", message.c_str());
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

void CommunicationHelper::setWebSocketHelper(WebSocketHelper* ws) {
    webSocketHelper = ws;
    Serial.println("[CommHelper] WebSocketHelper registered");
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
