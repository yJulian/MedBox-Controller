#include "wifi_helper.hpp"
#include "ble_helper.hpp"
#include <WiFi.h>
#include <defines.hpp>

// LED state codes for WiFi connection status
#define WIFI_CONNECTING_CODE 0xFF00  // Pattern during connection attempt
#define WIFI_CONNECTED_CODE  0x0303  // Pattern when successfully connected

WifiHelper::WifiHelper() {
    bleHelper = new BleHelper();
}

void WifiHelper::saveConfig(const String& ssid, const String& pass) {
    prefs.begin(namespaceName, false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();
}

bool WifiHelper::loadConfig(String& ssid, String& pass) {
    prefs.begin(namespaceName, true);
    ssid = prefs.getString("ssid", "");
    pass = prefs.getString("pass", "");
    prefs.end();
    return ssid.length() > 0;
}

bool WifiHelper::hasConfig() {
    String ssid, pass;
    return loadConfig(ssid, pass);
}

void WifiHelper::clearConfig() {
    prefs.begin(namespaceName, false);
    prefs.clear();
    prefs.end();
}

bool WifiHelper::connect() {
    String ssid, pass;

    // Check if reset button is pressed during boot
    if (digitalRead(RESET_PIN) == LOW) {
        Serial.println("[WiFi] Reset pin is LOW, clearing WiFi settings...");
        this->clearConfig();
    }

    // Load stored credentials
    if (!loadConfig(ssid, pass)) {
        Serial.println("[WiFi] No credentials found, starting BLE configuration...");
        startGattServer();
        return false;
    }

    // Set LED pattern to indicate connection attempt
    ledState = WIFI_CONNECTING_CODE;
    
    // Configure WiFi in station mode and begin connection
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Serial.printf("[WiFi] Attempting to connect to '%s'", ssid.c_str());

    // Wait up to 10 seconds for connection
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(300);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("[WiFi] Connected successfully! IP: ");
        Serial.println(WiFi.localIP());
        
        // Enable automatic reconnection on connection loss
        WiFi.setAutoReconnect(true);
        
        // Set LED pattern for successful connection
        ledState = WIFI_CONNECTED_CODE;
        return true;
    }

    Serial.println("[WiFi] Failed to establish connection.");
    return false;
}

void WifiHelper::startGattServer() {
    bleHelper->startServer(this);
}

void WifiHelper::loop() {
    bleHelper->loop();
}
