#ifndef WIFI_HELPER_HPP
#define WIFI_HELPER_HPP

#include <Arduino.h>
#include <Preferences.h>

class WifiHelper {
public:
    WifiHelper();

    bool connect();                                   // Verbindung aus NVS herstellen
    void saveConfig(const String& ssid, const String& pass); // WLAN speichern
    bool hasConfig();                                 // Existiert eine Config?
    void clearConfig();                               // WLAN löschen
    void loop();                                      // Muss in main loop aufgerufen werden
private:
    Preferences prefs;
    const char* namespaceName = "wifi";

    bool loadConfig(String& ssid, String& pass);      // Internes Laden
    void startGattServer();    
};

#endif // WIFI_HELPER_HPP
