#ifndef WIFI_HELPER_HPP
#define WIFI_HELPER_HPP

#include <Arduino.h>
#include <Preferences.h>

// Forward declaration
class BleHelper;

/**
 * @brief WiFi connection manager with BLE configuration fallback
 * 
 * Manages WiFi credentials in non-volatile storage (NVS) and handles
 * automatic connection. If no credentials exist or reset is triggered,
 * starts BLE GATT server for configuration via mobile app.
 */
class WifiHelper {
public:
    WifiHelper();

    /**
     * @brief Attempt WiFi connection using stored credentials
     * 
     * Checks for reset button (RESET_PIN LOW) to clear credentials.
     * If credentials exist, attempts connection with 10-second timeout.
     * On failure, starts BLE GATT server for configuration.
     * 
     * @return true if WiFi connected successfully, false otherwise
     */
    bool connect();
    
    /**
     * @brief Save WiFi credentials to non-volatile storage
     * @param ssid WiFi network name
     * @param pass WiFi password
     */
    void saveConfig(const String& ssid, const String& pass);
    
    /**
     * @brief Check if WiFi credentials exist in storage
     * @return true if credentials are stored
     */
    bool hasConfig();
    
    /**
     * @brief Clear stored WiFi credentials
     */
    void clearConfig();
    
    /**
     * @brief Process WiFi/BLE events (must be called in main loop)
     */
    void loop();

private:
    Preferences prefs;
    const char* namespaceName = "wifi";
    BleHelper* bleHelper;

    /**
     * @brief Load WiFi credentials from non-volatile storage
     * @param ssid Reference to store loaded SSID
     * @param pass Reference to store loaded password
     * @return true if credentials were loaded successfully
     */
    bool loadConfig(String& ssid, String& pass);
    
    /**
     * @brief Start BLE GATT server for WiFi configuration
     */
    void startGattServer();
};

#endif // WIFI_HELPER_HPP
