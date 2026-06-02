#include "ble_control_helper.hpp"
#include "message_parser.hpp"
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLE2902.h>
#include <defines.hpp>

// BLE connection status variables
static BLEServer* pControlServer = nullptr;
static BLECharacteristic* pControlCharacteristic = nullptr;
static bool controlDeviceConnected = false;
static bool oldControlDeviceConnected = false;
static BleControlHelper::BleControlCallback controlCallback = nullptr;

/**
 * @brief BLE Server callback handler for offline control mode
 */
class MyControlServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        controlDeviceConnected = true;
        ledState = 0xAAAA; // Pattern when BLE client is connected
        Serial.println("[BLE Control] Client connected");
    }
    
    void onDisconnect(BLEServer* pServer) override {
        controlDeviceConnected = false;
        ledState = 0xCCCC; // Pattern when BLE server is advertising
        Serial.println("[BLE Control] Client disconnected");
    }
};

/**
 * @brief BLE Characteristic callback handler for processing commands
 */
class MyControlCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            Serial.printf("[BLE Control] Received command: %s\n", rxValue.c_str());
            
            if (controlCallback != nullptr) {
                controlCallback(String(rxValue.c_str()));
                // Send feedback notification to client
                pControlCharacteristic->setValue("OK");
                pControlCharacteristic->notify();
            } else {
                Serial.println("[BLE Control] Error: Control callback not set!");
                pControlCharacteristic->setValue("ERROR: Callback Null");
                pControlCharacteristic->notify();
            }
        }
    }
};

BleControlHelper::BleControlHelper() {
    serverStarted = false;
}

void BleControlHelper::startServer(BleControlCallback callback) {
    controlCallback = callback;
    serverStarted = true;
    ledState = 0xCCCC; // Set status LED to BLE advertising pattern
    
    Serial.println("[BLE Control] Initializing BLE Device...");
    BLEDevice::init("MedBox BLE Control");

    pControlServer = BLEDevice::createServer();
    pControlServer->setCallbacks(new MyControlServerCallbacks());

    BLEService* pService = pControlServer->createService(BLE_CONTROL_SERVICE_UUID);

    pControlCharacteristic = pService->createCharacteristic(
        BLE_CONTROL_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );

    // Add descriptor required for notifications
    pControlCharacteristic->addDescriptor(new BLE2902());
    pControlCharacteristic->setCallbacks(new MyControlCharacteristicCallbacks());
    pControlCharacteristic->setValue("Ready for commands");

    pService->start();

    // Configure and start advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_CONTROL_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // Connection interval tuning
    pAdvertising->setMaxPreferred(0x12); // Connection interval tuning

    BLEDevice::startAdvertising();
    Serial.println("[BLE Control] Advertising started. Visible as 'MedBox BLE Control'");
}

void BleControlHelper::loop() {
    if (serverStarted) {
        // Handle client disconnection
        if (!controlDeviceConnected && oldControlDeviceConnected) {
            delay(500); // Give the BLE stack time to ready itself
            pControlServer->startAdvertising();
            ledState = 0xCCCC;
            Serial.println("[BLE Control] Restarted advertising");
            oldControlDeviceConnected = controlDeviceConnected;
        }

        // Handle client connection
        if (controlDeviceConnected && !oldControlDeviceConnected) {
            oldControlDeviceConnected = controlDeviceConnected;
        }
    }
}
