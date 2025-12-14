#ifndef WEBSOCKET_HELPER_HPP
#define WEBSOCKET_HELPER_HPP

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

/**
 * @brief WebSocket client helper for MedBox backend communication
 * 
 * Manages WebSocket connection with automatic reconnection and heartbeat.
 * Handles JSON message parsing and routing for backend commands.
 * 
 * Features:
 * - Automatic connection and reconnection (configurable interval)
 * - Keep-alive heartbeat mechanism (ping-pong)
 * - JSON message parsing with error handling
 * - Event-driven architecture for connection status
 */
class WebSocketHelper {
public:
    WebSocketHelper();
    
    /**
     * @brief Initialize and start WebSocket connection
     * 
     * Configures connection parameters from defines.hpp:
     * - WS_HOST, WS_PORT, WS_PATH: Server endpoint
     * - WS_RECONNECT_INTERVAL: Auto-reconnect timing
     * - WS_PING_INTERVAL: Keep-alive heartbeat
     */
    void begin();
    
    /**
     * @brief Process WebSocket events (must be called in main loop)
     * 
     * Handles incoming messages, connection state changes,
     * and automatic reconnection attempts.
     */
    void loop();
    
    /**
     * @brief Check current connection status
     * @return true if WebSocket is connected to server
     */
    bool isConnected();
    
    /**
     * @brief Send text message to WebSocket server
     * @param message String message to send
     */
    void sendMessage(const String& message);
    
private:
    WebSocketsClient webSocket;
    bool connected;
    
    /**
     * @brief Internal event handler for WebSocket events
     * 
     * Processes all WebSocket event types:
     * - Connection/disconnection
     * - Text/binary data
     * - Ping/pong keep-alive
     * - Errors
     * 
     * @param type Event type (WStype_t enum)
     * @param payload Event data
     * @param length Data length in bytes
     */
    void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length);
    
    /**
     * @brief Parse and handle JSON messages from server
     * 
     * Safely extracts common fields (type, command, value) and
     * routes to appropriate handlers. Designed to be extended
     * as protocol evolves.
     * 
     * @param doc Parsed JSON document
     */
    void handleJsonMessage(const JsonDocument& doc);
    
    /**
     * @brief Static instance pointer for lambda callback
     * 
     * Required because WebSocketsClient uses C-style callbacks.
     */
    static WebSocketHelper* instance;
};

#endif // WEBSOCKET_HELPER_HPP
