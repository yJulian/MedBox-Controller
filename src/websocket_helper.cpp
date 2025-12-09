#include "websocket_helper.hpp"
#include <defines.hpp>

WebSocketHelper* WebSocketHelper::instance = nullptr;

WebSocketHelper::WebSocketHelper() {
    connected = false;
    lastPingTime = 0;
    lastReconnectAttempt = 0;
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
    
    // Additional ping handling if needed
    if (connected) {
        unsigned long currentTime = millis();
        if (currentTime - lastPingTime >= WS_PING_INTERVAL) {
            // WebSocket library handles ping automatically with heartbeat
            lastPingTime = currentTime;
        }
    }
}

bool WebSocketHelper::isConnected() {
    return connected;
}

void WebSocketHelper::sendMessage(const String& message) {
    if (connected) {
        webSocket.sendTXT(message);
        Serial.printf("[WS] Sent message: %s\n", message.c_str());
    } else {
        Serial.println("[WS] Cannot send message - not connected");
    }
}

void WebSocketHelper::onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WS] Disconnected!");
            connected = false;
            break;
            
        case WStype_CONNECTED:
            Serial.printf("[WS] Connected to url: %s\n", payload);
            connected = true;
            lastPingTime = millis();
            
            // Send initial message to server
            webSocket.sendTXT("ESP32 Connected");
            break;
            
        case WStype_TEXT:
            Serial.printf("[WS] Received text: %s\n", payload);
            // Handle incoming messages here
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

void WebSocketHelper::reconnect() {
    unsigned long currentTime = millis();
    if (currentTime - lastReconnectAttempt >= WS_RECONNECT_INTERVAL) {
        Serial.println("[WS] Attempting to reconnect...");
        lastReconnectAttempt = currentTime;
        // WebSocket library handles reconnection automatically
    }
}
