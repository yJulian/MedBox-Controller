#include "ble_helper.hpp"
#include "wifi_helper.hpp"
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLE2902.h>
#include <defines.hpp>

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcdefab-1234-5678-9abc-def012345678"

// LED state codes for different BLE states
#define GATT_SERVER_STARTED_CODE   0xCCCC  // Pattern when BLE server is advertising
#define GATT_SERVER_CONNECTED_CODE 0xAAAA  // Pattern when client is connected
#define GATT_WIFI_CONNECTION_TRY   0xFF00  // Pattern during WiFi connection attempt

// Global state for BLE server
// Note: Using globals due to BLE library callback constraints
static BLEServer* pServer = nullptr;
static BLECharacteristic* pCharacteristic = nullptr;
static bool deviceConnected = false;
static bool oldDeviceConnected = false;
static bool restart = false;

/**
 * @brief BLE Server callback handler
 * 
 * Tracks client connection status and updates LED state accordingly.
 */
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
        ledState = GATT_SERVER_CONNECTED_CODE;
        Serial.println("[BLE] Client connected");
    }
    
    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
        ledState = GATT_SERVER_STARTED_CODE;
        Serial.println("[BLE] Client disconnected");
    }
};

/**
 * @brief BLE Characteristic callback handler
 * 
 * Implements a state machine to handle WiFi configuration:
 * State 0: Idle - waiting for commands (GET_MAC, SCAN_WIFI, CON_WIFI)
 * State 1: Credentials - collecting SSID and password
 * 
 * Command protocol:
 * - "GET_MAC": Returns device MAC address
 * - "SCAN_WIFI": Scans and sends list of available WiFi networks
 * - "CON_WIFI": Initiates WiFi credential collection
 * - "S<ssid>": Sets SSID (state 1)
 * - "P<password>": Sets password (state 1)
 * 
 * After receiving both SSID and password, attempts WiFi connection
 * and saves credentials on success.
 */
class MyCallbacks : public BLECharacteristicCallbacks {
    WifiHelper* pWifiHelper;
    
public:
    MyCallbacks(WifiHelper* wifiHelper) : pWifiHelper(wifiHelper) {}
    
    void onWrite(BLECharacteristic* pCharacteristic) override {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            if (state == 0) {
                // State 0: Idle - process commands
                if (rxValue == "GET_MAC") {
                    String mac = "MAC:" + WiFi.macAddress();
                    sendChunk(mac.c_str());
                }
                if (rxValue == "SCAN_WIFI") {
                    this->scanWifi();
                }
                if (rxValue == "CON_WIFI") {
                    state = 1; // Transition to credential collection state
                    Serial.println("[BLE] Received CON_WIFI, waiting for SSID...");
                }
            } else if (state == 1) {
                // State 1: Credential collection - waiting for SSID and password
                char type = rxValue.front();
                if (type == 'S') {
                    // SSID received
                    WifiSSID = String(rxValue.substr(1).c_str());
                    Serial.printf("[BLE] Received SSID: %s\n", WifiSSID.c_str());
                } else if (type == 'P') {
                    // Password received
                    WifiPass = String(rxValue.substr(1).c_str());
                    Serial.printf("[BLE] Received Password: %s\n", WifiPass.c_str());
                }
                
                // Once both credentials are received, attempt connection
                if (WifiSSID.length() > 0 && WifiPass.length() > 0) {
                    Serial.println("[BLE] Both SSID and Password received, attempting to connect...");
                    ledState = GATT_WIFI_CONNECTION_TRY;
                    
                    WiFi.mode(WIFI_STA);
                    WiFi.begin(WifiSSID.c_str(), WifiPass.c_str());

                    Serial.printf("[BLE] Trying to connect to %s", WifiSSID.c_str());

                    // Wait up to 10 seconds for connection
                    unsigned long start = millis();
                    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
                        delay(300);
                        Serial.print(".");
                    }
                    
                    if (WiFi.status() == WL_CONNECTED) {
                        // Connection successful - save and restart
                        sendChunk("SUCCESS");
                        Serial.println("[BLE] Connected to WiFi successfully, saving config...!");
                        pWifiHelper->saveConfig(WifiSSID, WifiPass);
                        Serial.println("[BLE] Configuration saved. Restarting to connect...");

                        restart = true; // Schedule restart in loop()
                    } else {
                        // Connection failed - reset and retry
                        WifiSSID = "";
                        WifiPass = "";
                        sendChunk("FAILED");
                        state = 0; // Return to idle state
                        Serial.println("[BLE] Failed to connect to WiFi with provided credentials.");
                    }
                }
            }        
        }
    }
    
private:
    int state = 0;        // State machine: 0=idle, 1=collecting credentials
    String WifiSSID;      // Temporary storage for SSID
    String WifiPass;      // Temporary storage for password

    /**
     * @brief Send a message chunk to BLE client via notification
     */
    void sendChunk(const char *value) {
        pCharacteristic->setValue(value);
        pCharacteristic->notify(); 
    }

    /**
     * @brief Scan for WiFi networks and send results to client
     * 
     * Sends results as a sequence of notifications:
     * 1. "Begin Wifi" marker
     * 2. One notification per network: "SSID:<name>,RSSI:<signal>"
     * 3. "End Wifi" marker
     */
    void scanWifi() {
        Serial.println("[BLE] Scanning WiFi networks...");
        int n = WiFi.scanNetworks();

        sendChunk("Begin Wifi");

        // Send each network as a separate notification
        for (int i = 0; i < n; i++) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "SSID:%s,RSSI:%d", 
                     WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            sendChunk(buffer);
        }

        sendChunk("End Wifi");
        Serial.printf("[BLE] Found %d networks\n", n);
    }
};

BleHelper::BleHelper() {
    gattServerStarted = false;
}

void BleHelper::startServer(WifiHelper* wifiHelper) {
    ledState = GATT_SERVER_STARTED_CODE;
    gattServerStarted = true;
    Serial.println("[BLE] Starting GATT server...");
    
    // Initialize BLE module with device name
    BLEDevice::init("MedBox Controller");

    // Create GATT Server with connection callbacks
    // Note: BLE library takes ownership of callback object
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create service for WiFi configuration
    BLEService* pService = pServer->createService(SERVICE_UUID);

    // Create characteristic with read, write, notify and indicate properties
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );

    // Add descriptor required for notifications in most BLE apps
    // Note: BLE library takes ownership of descriptor
    pCharacteristic->addDescriptor(new BLE2902());

    // Set callback for handling writes from client
    // Note: BLE library takes ownership of callback object
    pCharacteristic->setCallbacks(new MyCallbacks(wifiHelper));

    // Set initial value
    pCharacteristic->setValue("Hello from ESP32");

    // Start the service
    pService->start();

    // Configure and start advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Connection interval tuning
    pAdvertising->setMaxPreferred(0x12);  // Connection interval tuning

    BLEDevice::startAdvertising();
    Serial.println("[BLE] Advertising started - device visible as 'MedBox Controller'");
}

void BleHelper::loop() {
    if (gattServerStarted) {
        // Handle connection status changes
        if (!deviceConnected && oldDeviceConnected) {
            // Client just disconnected - restart advertising
            pServer->startAdvertising();
            ledState = GATT_SERVER_STARTED_CODE;
            Serial.println("[BLE] Start advertising again");
            oldDeviceConnected = deviceConnected;
        }

        if (deviceConnected && !oldDeviceConnected) {
            // Client just connected
            oldDeviceConnected = deviceConnected;
        }

        // Restart ESP if WiFi configuration was successful
        if (restart) {
            Serial.println("[BLE] Restarting ESP32...");
            delay(1000);
            ESP.restart();
        }
    }
}
