#pragma once
#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

class WebSocketHelper {
public:
    WebSocketHelper();
    
    void begin();                    // Initialize WebSocket connection
    void loop();                     // Must be called in main loop
    bool isConnected();              // Check if WebSocket is connected
    void sendMessage(const String& message);  // Send message to server
    
private:
    WebSocketsClient webSocket;
    bool connected;
    
    void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length);
    void handleJsonMessage(const JsonDocument& doc);
    
    static WebSocketHelper* instance;  // For static callback
};
