#ifndef WEBSOCKET_HELPER_HPP
#define WEBSOCKET_HELPER_HPP

#include <Arduino.h>
#include <WebSocketsClient.h>

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
    static void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
    
    static WebSocketHelper* instance;  // For static callback
};

#endif // WEBSOCKET_HELPER_HPP
