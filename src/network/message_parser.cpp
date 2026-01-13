#include "message_parser.hpp"

MessageParser::MessageParser(String targetMac) : targetBoxMac(targetMac) {
    // Constructor implementation (if needed)
}

void MessageParser::parseMessage(const String& message) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, message);

    if (err) {
        Serial.printf("[MessageParser] JSON parse error: %s\n", err.c_str());
        return;
    }
    // Successfully parsed - handle the JSON message
    #ifdef DEBUG
    Serial.println("[MessageParser] Successfully parsed message:");
    #endif

    String targetMac = doc["targetBoxMac"].as<String>();
    if (targetMac != targetBoxMac) {
        #ifdef DEBUG
        Serial.printf("Target: '%s', This Box: '%s'\n", targetMac.c_str(), targetBoxMac.c_str());
        #endif
        return; // Message not intended for this box
    }

    switch (doc["messageType"].as<uint16_t>()) {
        case 3: { // Command Message
            int compartment = doc["message"]["compartmentNumber"];
            int amount = doc["message"]["amountOfPillsToDispense"];

            //todo : Implement command handling logic
        }
    }
}