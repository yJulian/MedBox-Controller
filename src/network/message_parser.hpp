#pragma once

#include <HardwareSerial.h>
#include <ArduinoJson.h>

class CompartmentSet;  // Forward declaration

class MessageParser {
public:
    
    /**
     * @brief Set the compartment set for dispensing commands
     * @param compartmentSet Pointer to the CompartmentSet instance
     */
    void setCompartmentSet(CompartmentSet* compartmentSet);
    
    /**
     * @brief Parse incoming message string
     * @param message Raw message string to parse
     * @return true if parsing was successful, false otherwise
     */
    void parseMessage(const String& message);

    void setTargetBoxMac(const String& mac);

private:
    String targetBoxMac;
    CompartmentSet* compartmentSet = nullptr;
};