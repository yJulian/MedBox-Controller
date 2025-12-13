#ifndef BLE_HELPER_HPP
#define BLE_HELPER_HPP

#include <Arduino.h>

// Forward declaration for WifiHelper to avoid circular dependency
class WifiHelper;

/**
 * @brief BLE GATT Server helper for WiFi configuration
 * 
 * Provides a BLE GATT interface for:
 * - Scanning available WiFi networks
 * - Receiving WiFi credentials from a mobile app
 * - Testing WiFi connection before saving
 * - Retrieving device MAC address
 */
class BleHelper {
public:
    BleHelper();
    
    /**
     * @brief Start the BLE GATT server
     * 
     * Creates a BLE server with service UUID and characteristic
     * for WiFi configuration. Starts advertising as "MedBox Controller".
     * 
     * @param wifiHelper Pointer to WifiHelper for saving credentials
     */
    void startServer(WifiHelper* wifiHelper);
    
    /**
     * @brief Process BLE server events in main loop
     * 
     * Handles connection status changes and triggers ESP restart
     * after successful WiFi configuration.
     */
    void loop();
    
    /**
     * @brief Check if GATT server is running
     * @return true if server has been started
     */
    bool isServerStarted() const { return gattServerStarted; }

private:
    bool gattServerStarted;
};

#endif // BLE_HELPER_HPP
