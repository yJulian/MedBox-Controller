#ifndef WIFI_HELPER_CPP
#define WIFI_HELPER_CPP

#include <defines.hpp>
#include <WiFiManager.h> 
#include <wifi_helper.hpp>

bool connectWifi() {
    WiFiManager wifiManager;

  // Check if the reset pin is LOW to reset WiFi settings
  if (digitalRead(RESET_PIN) == LOW) {
    Serial.println("Reset pin is LOW, resetting WiFi settings...");
    
    wifiManager.resetSettings();
    delay(1000); // Wait a moment to ensure settings are cleared
  }
    
    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res = wifiManager.autoConnect(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

    return res;
}
#endif // WIFI_HELPER_CPP