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
    
    // Configure WebSocket connection endpoint for plain ws (no TLS)
    // Ensure WS_HOST matches server binding and WS_PATH matches endpoint
    const String path = WS_PATH + WiFi.macAddress();
    webSocket.begin(WS_HOST, WS_PORT, path);

    // Register event handler using lambda to bridge C-style callback
    webSocket.onEvent([](WStype_t type, uint8_t* payload, size_t length) {
        // Verbose event pre-log for diagnostics
        if (instance) {
            instance->onWebSocketEvent(type, payload, length);
        }
    });
    
    // Enable heartbeat mechanism to detect dead connections
    // Parameters: ping_interval_ms, pong_timeout_ms, disconnect_timeout_count
    webSocket.enableHeartbeat(WS_PING_INTERVAL, 3000, 2);
    Serial.printf("[WS] Heartbeat enabled: ping=%ums, pong-timeout=%ums, max-missed=%u\n", (unsigned)WS_PING_INTERVAL, 3000u, 2u);
    
    // Configure automatic reconnection on disconnect
    webSocket.setReconnectInterval(WS_RECONNECT_INTERVAL);
    Serial.printf("[WS] Reconnect interval set to %ums\n", (unsigned)WS_RECONNECT_INTERVAL);
    
    Serial.printf("[WS] Configured to connect to ws://%s:%d%s\n", WS_HOST, WS_PORT, path);
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
            Serial.println("[WS] Disconnected from server! Will attempt reconnect.");
            ledState = 0xFF00;  // Set LED pattern to indicate disconnection
            connected = false;
            break;
            
        case WStype_CONNECTED:
            Serial.printf("[WS] Connected to server. URL echo: %s\n", payload);
            ledState = 0x0000;  // Set LED pattern for active connection (solid on)
            connected = true;

            // todo: Send initial status or registration message if needed
            break;
            
        case WStype_TEXT:
            Serial.printf("[WS] Received text message (len=%u): %s\n", (unsigned)length, payload);
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
            Serial.printf("[WS] Received binary data, length: %u bytes\n", (unsigned)length);
            // Binary protocol not yet implemented
            // TODO: Add binary message handling if needed
            break;
            
        case WStype_PING:
            break;
            
        case WStype_PONG:
            break;
            
        case WStype_ERROR:
            Serial.printf("[WS] Error occurred (len=%u): %s\n", (unsigned)length, payload ? (const char*)payload : "<null>");
            connected = false;
            break;
            
        default:
            Serial.printf("[WS] Unknown event type: %d (len=%u)\n", (int)type, (unsigned)length);
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
}
