#include "websocket_helper.hpp"
#include <defines.hpp>

// Static instance for callback access
WebSocketHelper* WebSocketHelper::instance = nullptr;

WebSocketHelper::WebSocketHelper() {
    connected = false;
    instance = this;
}

void WebSocketHelper::begin() {
    Serial.println("[WS] Initializing WebSocket connection...");
    
    // Configure WebSocket connection endpoint (TLS - wss)
    webSocket.begin(WS_HOST, WS_PORT, WS_PATH);


    // Register event handler using lambda to bridge C-style callback
    webSocket.onEvent([](WStype_t type, uint8_t* payload, size_t length) {
        if (instance) {
            instance->onWebSocketEvent(type, payload, length);
        }
    });
    
    // Enable heartbeat mechanism to detect dead connections
    // Parameters: ping_interval_ms, pong_timeout_ms, disconnect_timeout_count
    webSocket.enableHeartbeat(WS_PING_INTERVAL, 3000, 2);
    
    // Configure automatic reconnection on disconnect
    webSocket.setReconnectInterval(WS_RECONNECT_INTERVAL);
    
    Serial.printf("[WS] Configured to connect to ws://%s:%d/%s\n", WS_HOST, WS_PORT, WS_PATH);
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
            Serial.println("[WS] Disconnected from server!");
            ledState = 0xFF00;  // Set LED pattern to indicate disconnection
            connected = false;
            break;
            
        case WStype_CONNECTED:
            Serial.printf("[WS] Connected to server: %s\n", payload);
            ledState = 0x0000;  // Set LED pattern for active connection (solid on)
            connected = true;
            
            // Send handshake message to backend
            webSocket.sendTXT("ESP32 Connected");
            break;
            
        case WStype_TEXT:
            Serial.printf("[WS] Received text message: %s\n", payload);
            {
                // Parse incoming JSON message
                JsonDocument doc;
                DeserializationError err = deserializeJson(doc, payload, length);
                if (err) {
                    Serial.printf("[WS] JSON parse error: %s (message length: %u)\n", 
                                  err.c_str(), (unsigned)length);
                } else {
                    // Successfully parsed - route to handler
                    handleJsonMessage(doc);
                }
            }
            break;
            
        case WStype_BIN:
            Serial.printf("[WS] Received binary data, length: %u bytes\n", length);
            // Binary protocol not yet implemented
            // TODO: Add binary message handling if needed
            break;
            
        case WStype_PING:
            Serial.println("[WS] Received ping (heartbeat check)");
            break;
            
        case WStype_PONG:
            Serial.println("[WS] Received pong (heartbeat response)");
            break;
            
        case WStype_ERROR:
            Serial.printf("[WS] Error occurred: %s\n", payload);
            connected = false;
            break;
            
        default:
            Serial.printf("[WS] Unknown event type: %d\n", type);
            break;
    }
}

void WebSocketHelper::handleJsonMessage(const JsonDocument& doc) {
    /**
     * JSON Message Handler
     * 
     * Safely extracts common fields using default values to prevent crashes.
     * Current protocol expects: {"type": "...", "command": "...", "value": ...}
     * 
     * TODO: Implement specific command handlers:
     * - Motor control commands
     * - Status query responses
     * - Configuration updates
     */
    
    // Extract fields with safe defaults (using | operator)
    const char* type = doc["type"] | "unknown";
    const char* command = doc["command"] | "none";
    int value = doc["value"] | 0;

    Serial.printf("[WS] Parsed JSON -> type: '%s', command: '%s', value: %d\n", 
                  type, command, value);

    // Route to specific handlers based on message type
    // Extend this section as backend protocol evolves
    
    // Example routing (to be implemented):
    // if (strcmp(type, "motor") == 0) {
    //     handleMotorCommand(command, value);
    // } else if (strcmp(type, "status") == 0) {
    //     handleStatusRequest(command);
    // }
}
