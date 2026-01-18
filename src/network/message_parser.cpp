#include "message_parser.hpp"
#include "../motor/compartment_set.hpp"

void MessageParser::setCompartmentSet(CompartmentSet* compartmentSet) {
    this->compartmentSet = compartmentSet;
    #ifdef DEBUG
    Serial.println("[MessageParser] CompartmentSet assigned");
    #endif
}

void MessageParser::setTargetBoxMac(const String& mac) {
    Serial.println("[MessageParser] Target Box MAC set to: " + mac);
    targetBoxMac = mac;
    Serial.println("[MessageParser] Current Target Box MAC: " + targetBoxMac);
}

void MessageParser::parseMessage(const String& message) {
    Serial.println("[WebSocket 2] Message received, parsing...");
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, message);

    Serial.println("[WebSocket 3] Message received, parsing...");
    if (err) {
        Serial.printf("[MessageParser] JSON parse error: %s\n", err.c_str());
        return;
    }
    // Successfully parsed - handle the JSON message
    #ifdef DEBUG
    Serial.println("[MessageParser] Successfully parsed message:");
    #endif

    String targetMac = doc["targetBoxMAC"].as<String>();
    Serial.printf("Target: '%s', This Box: '%s'\n", targetMac.c_str(), targetBoxMac.c_str());
    if (targetMac != targetBoxMac) {
        #ifdef DEBUG
        Serial.printf("Target: '%s', This Box: '%s'\n", targetMac.c_str(), targetBoxMac.c_str());
        #endif
        Serial.println("[MessageParser] Message not intended for this box, ignoring.");
        return; // Message not intended for this box
    }
    Serial.println("[MessageParser] Message intended for this box, processing...");

    const int messageType = doc["messageType"];
    Serial.printf("[MessageParser] Message type: %d\n", messageType);
    switch (messageType) {
        case 3: { // Command Message
            int compartment = doc["message"]["compartmentNumber"];
            int amount = doc["message"]["amountOfPillsToDispense"];

            if (compartmentSet != nullptr) {
                #ifdef DEBUG
                Serial.printf("[MessageParser] Dispensing %d pills from compartment %d\n", amount, compartment);
                #endif
                compartmentSet->dispense(compartment, amount);
            } else {
                Serial.println("[MessageParser] ERROR: CompartmentSet not initialized!");
            }
            break;
        }
    }
}