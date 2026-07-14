#ifndef BLE_CONTROL_HELPER_HPP
#define BLE_CONTROL_HELPER_HPP

#include <Arduino.h>

class MessageParser; // Forward declaration

#include <functional>

/**
 * @brief BLE GATT Server helper for offline motor control
 * 
 * Provides a BLE GATT interface to control motors directly by parsing 
 * incoming JSON/command payloads via a callback.
 */
class BleControlHelper {
public:
    typedef std::function<void(const String&)> BleControlCallback;

    BleControlHelper();
    
    /**
     * @brief Start the BLE Control GATT server
     * 
     * Creates a BLE server with service and characteristic UUIDs for motor control.
     * Starts advertising as "MedBox BLE Control".
     * 
     * @param callback Callback function triggered when a control command is received
     */
    void startServer(BleControlCallback callback);
    
    /**
     * @brief Process BLE server events in main loop
     */
    void loop();
    
    /**
     * @brief Check if GATT server is running
     * @return true if server has been started
     */
    bool isServerStarted() const { return serverStarted; }

private:
    bool serverStarted;
};

#endif // BLE_CONTROL_HELPER_HPP
