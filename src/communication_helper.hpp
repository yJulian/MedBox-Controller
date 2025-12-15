#ifndef COMMUNICATION_HELPER_HPP
#define COMMUNICATION_HELPER_HPP

#include <Arduino.h>
#include <HardwareSerial.h>
#include <functional>
#include "defines.hpp"

/**
 * @brief Communication helper for inter-MedBox communication
 * 
 * Manages three types of communication between MedBox controllers:
 * 1. UART: Standard UART on Serial2 (TX_PIN 17, RX_PIN 16)
 * 2. Serial pins: Chain communication with interrupt-driven input
 * 3. Parallel pin: Wired-AND broadcast communication
 * 
 * All communication uses pins defined in defines.hpp.
 */
class CommunicationHelper {
public:
    /**
     * @brief Callback type for received UART data
     * @param data Received string from UART
     */
    using UartCallback = std::function<void(const String& data)>;
    
    /**
     * @brief Callback type for serial input interrupt
     * @param state Current state of SERIAL_IN_PIN (HIGH or LOW)
     */
    using SerialInputCallback = std::function<void(int state)>;

    struct SlaveInfo {
        uint16_t idx;
        String mac;
    };

    // Example array size, adjust as needed
    SlaveInfo slaves[MAX_SLAVES];

    CommunicationHelper();
    
    /**
     * @brief Initialize all communication interfaces
     * 
     * Sets up:
     * - UART on Serial2 with 115200 baud rate
     * - SERIAL_IN_PIN as INPUT with interrupt
     * - SERIAL_OUT_PIN as OUTPUT (initially HIGH)
     * - PARALLEL_PIN as INPUT (high-impedance for wired-AND)
     */
    void begin(bool isMaster);
    
    /**
     * @brief Process incoming UART data (must be called in main loop)
     * 
     * Checks for available UART data and triggers callback
     * if data is received.
     */
    void loop();
    
    // ========================================================================
    // UART Communication
    // ========================================================================
    
    /**
     * @brief Send string via UART
     * @param message String to send to other MedBox controllers
     */
    void sendUart(const String& message);
    
    /**
     * @brief Set callback for received UART data
     * @param callback Function to call when UART data is received
     */
    void setUartCallback(UartCallback callback);
    
    // ========================================================================
    // Serial Pin Communication
    // ========================================================================
    
    /**
     * @brief Send short pulse on SERIAL_OUT_PIN
     * 
     * Pulls SERIAL_OUT_PIN LOW for a brief moment (1ms),
     * then returns it to HIGH state.
     */
    void pulseSerialOut(uint32_t delayUs = 1000);
    
    /**
     * @brief Set callback for SERIAL_IN_PIN interrupt
     * @param callback Function to call when SERIAL_IN_PIN changes state
     */
    void setSerialInputCallback(SerialInputCallback callback);
    
    // ========================================================================
    // Parallel Pin Communication (Wired-AND)
    // ========================================================================
    
    /**
     * @brief Send short pulse on PARALLEL_PIN using wired-AND
     * 
     * Temporarily configures PARALLEL_PIN as OUTPUT, pulls it to GND
     * for a brief moment (1ms) to send pulse, then releases it back
     * to INPUT (high-impedance Z-state) for wired-AND operation.
     * 
     * In wired-AND configuration, multiple devices can pull the line LOW,
     * but the line only goes HIGH when all devices release it.
     */
    void pulseParallelPin();

    void beginUartEnumeration();
    
private:
    HardwareSerial uart;
    UartCallback uartCallback;
    SerialInputCallback serialInputCallback;

    enum State {
        NORMAL,
        ENUMERATION
    } state;
    bool isMaster;    
    String uartBuffer;
    
    // ISR flag and state for deferred processing
    volatile bool serialInputChanged;
    volatile int serialInputState;

    using enumerationUartHandler = std::function<void(const String& data)>;
    void enumerationUartMasterHandler(const String& data);
    void enumerationUartSlaveHandler(const String& data);

    /**
     * @brief Static instance pointer for ISR callbacks
     * 
     * Required because Arduino ISRs require static/global functions.
     */
    static CommunicationHelper* instance;
    
    /**
     * @brief Static ISR handler for SERIAL_IN_PIN interrupt
     * 
     * Sets flag and reads pin state for processing in main loop.
     */
    static void IRAM_ATTR serialInputISR();

    uint16_t currentSlaveIdx = 0;

    unsigned long lastEnumerationTime = 0;

    void handleSlaveEnumerationRequest();

    bool waitForNextRequest = false;
};

#endif // COMMUNICATION_HELPER_HPP
