#pragma once

#include <HardwareSerial.h>
#include <ArduinoJson.h>

class MessageParser {
public:
    MessageParser(String targetMac);
    
    /**
     * @brief Parse incoming message string
     * @param message Raw message string to parse
     * @return true if parsing was successful, false otherwise
     */
    void parseMessage(const String& message);

private:
    String targetBoxMac;
};