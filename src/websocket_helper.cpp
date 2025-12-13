#include "websocket_helper.hpp"
#include <defines.hpp>

WebSocketHelper* WebSocketHelper::instance = nullptr;

WebSocketHelper::WebSocketHelper() {
    connected = false;
    instance = this;
}

void WebSocketHelper::begin() {
    Serial.println("[WS] Initializing WebSocket connection...");
    
    // Configure WebSocket connection
    webSocket.begin(WS_HOST, WS_PORT, WS_PATH);
    
    // Set event handler
    webSocket.onEvent([](WStype_t type, uint8_t* payload, size_t length) {
        if (instance) {
            instance->onWebSocketEvent(type, payload, length);
        }
    });
    
    // Enable heartbeat (ping-pong) to keep connection alive
    webSocket.enableHeartbeat(WS_PING_INTERVAL, 3000, 2);
    
    // Set reconnect interval
    webSocket.setReconnectInterval(WS_RECONNECT_INTERVAL);
    
    Serial.printf("[WS] Configured to connect to ws://%s:%d%s\n", WS_HOST, WS_PORT, WS_PATH);
}

void WebSocketHelper::loop() {
    webSocket.loop();
}

bool WebSocketHelper::isConnected() {
    return connected;
}

void WebSocketHelper::sendMessage(const String& message) {
    if (connected) {
        webSocket.sendTXT((uint8_t *)message.c_str(), message.length());
        Serial.printf("[WS] Sent message: %s\n", message.c_str());
    } else {
        Serial.println("[WS] Cannot send message - not connected");
    }
}

void WebSocketHelper::onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WS] Disconnected!");
            ledState = 0xFF00;
            connected = false;
            break;
            
        case WStype_CONNECTED:
            Serial.printf("[WS] Connected to url: %s\n", payload);
            ledState = 0x0000;
            connected = true;
            
            // Send initial message to server
            webSocket.sendTXT("ESP32 Connected");
            break;
            
        case WStype_TEXT:
            Serial.printf("[WS] Received text: %s\n", payload);
            {
                JsonDocument doc;
                DeserializationError err = deserializeJson(doc, payload, length);
                if (err) {
                    Serial.printf("[WS] JSON parse error: %s (len=%u)\n", err.c_str(), (unsigned)length);
                } else {
                    handleJsonMessage(doc);
                }
            }
            break;
            
        case WStype_BIN:
            Serial.printf("[WS] Received binary data, length: %u\n", length);
            // Handle binary data here
            break;
            
        case WStype_PING:
            Serial.println("[WS] Received ping");
            break;
            
        case WStype_PONG:
            Serial.println("[WS] Received pong");
            break;
            
        case WStype_ERROR:
            Serial.printf("[WS] Error: %s\n", payload);
            connected = false;
            break;
            
        default:
            break;
    }
}

void WebSocketHelper::handleJsonMessage(const JsonDocument& doc) {
    // Example: read common fields safely and log them.
    // Adjust keys as your backend schema evolves.
    const char* type = doc["type"] | "unknown";
    const char* command = doc["command"] | "none";
    int value = doc["value"] | 0;

    Serial.printf("[WS] JSON message -> type: %s, command: %s, value: %d\n", type, command, value);

    // Implement routing based on parsed content without throwing exceptions.
    // For now, only logging; extend with actual control logic as needed.
}
