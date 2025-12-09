#include "wifi_helper.hpp"
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLE2902.h>
#include <defines.hpp>

WifiHelper::WifiHelper() {
    // leer für jetzt
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

    if (digitalRead(RESET_PIN) == LOW) {
        Serial.println("Reset pin is LOW, resetting WiFi settings...");
        prefs.begin(namespaceName, false);
        prefs.clear();   // löscht alle Keys im Namespace "wifi"
        prefs.end();
    }

    if (!loadConfig(ssid, pass)) {
        startGattServer();
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Serial.printf("Trying to connect to %s", ssid.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(300);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Wifi Connected: ");
        Serial.println(WiFi.localIP());
        WiFi.setAutoReconnect(true);
        return true;
    }

    Serial.println("Could not establish WiFi connection.");
    return false;
}

// Eigene UUIDs (kannst du mit irgendeinem UUID-Generator ersetzen)
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcdefab-1234-5678-9abc-def012345678"
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t valueCounter = 0;
    
// Server Callback -> Connection Status tracken
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("[BLE] Client connected");
    }
    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("[BLE] Client disconnected");
    }
};

struct WifiNetwork {
    String ssid;
    int32_t rssi;
};


// Characteristic Callback -> reagiert auf Writes vom Client
class MyCallbacks : public BLECharacteristicCallbacks {
    WifiHelper* pWifiHelper;
    
public:
    MyCallbacks(WifiHelper* wifiHelper) : pWifiHelper(wifiHelper) {}
    
    void onWrite(BLECharacteristic* pCharacteristic) override {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            if (state == 0) {
                if (rxValue == "SCAN_WIFI") {
                    this->scanWifi();
                }
                if (rxValue == "CON_WIFI") {
                    state = 1; // Next state: wait for SSID
                    Serial.println("Received CON_WIFI, waiting for SSID...");
                }
            } else if (state == 1) // Waiting for Wifi SSID
            {
                char type = rxValue.front();
                if (type == 'S') {
                    WifiSSID = String(rxValue.substr(1).c_str());
                    Serial.printf("Received SSID: %s\n", WifiSSID.c_str());
                } else if (type == 'P') {
                    WifiPass = String(rxValue.substr(1).c_str());
                    Serial.printf("Received Password: %s\n", WifiPass.c_str());
                }
                if (WifiSSID.length() > 0 && WifiPass.length() > 0) {
                    // Both SSID and Password received
                    Serial.println("Both SSID and Password received, saving config...");
                    pWifiHelper->saveConfig(WifiSSID, WifiPass);
                    Serial.println("Configuration saved. Restarting to connect...");
                    ESP.restart();
                }
            }        
        }
    }
private:
    int state = 0;
    void sendChunk(const char *value) {
        pCharacteristic->setValue(value);
        pCharacteristic->notify();  // Notify an Client
    }
    String WifiSSID;
    String WifiPass;

    void scanWifi() {
        Serial.println("Scanning Wifi networks...");
        int n = WiFi.scanNetworks();

        String networks;

        sendChunk("Begin Wifi");

        for (int i = 0; i < n; i++) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "SSID:%s,RSSI:%d", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            sendChunk(buffer);
        }

        sendChunk("End Wifi");

        Serial.println(networks);
    }
};


void WifiHelper::startGattServer() {
    // 1) BLE initialisieren
    BLEDevice::init("MedBox Controller");  // Name, den du in der App siehst

    // 2) Server erstellen
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // 3) Service erstellen
    BLEService* pService = pServer->createService(SERVICE_UUID);

    // 4) Characteristic erstellen
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );

    // 5) Descriptor hinzufügen (wichtig für Notify in vielen Apps)
    pCharacteristic->addDescriptor(new BLE2902());

    // 6) Callback setzen
    pCharacteristic->setCallbacks(new MyCallbacks(this));

    // Initialer Wert
    pCharacteristic->setValue("Hello from ESP32");

    // 7) Service starten
    pService->start();

    // 8) Advertising starten
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // optional, tuning
    pAdvertising->setMaxPreferred(0x12);  // optional, tuning

    BLEDevice::startAdvertising();
    Serial.println("[BLE] Advertising started");
}

void WifiHelper::loop() {
    // Connection-Status Änderungen handeln (neu werben nach Disconnect)
    if (!deviceConnected && oldDeviceConnected) {
        // kurz warten, sonst meckern manche Geräte
        delay(500);
        pServer->startAdvertising();
        Serial.println("[BLE] Start advertising again");
        oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected) {
        // frisch verbunden
        oldDeviceConnected = deviceConnected;
    }

    loop();
}
