#pragma once

#include "pill_dispenser.hpp"
#include "rotary_funnel.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// CompartmentSet runs all motor movement (dispensing, funnel rotation) on its
// own FreeRTOS task pinned to a dedicated core, so that callers (e.g. the BLE
// write callback, which runs on the same core as the BLE/WiFi protocol stack)
// never block on the motors and stall the network connection.
class CompartmentSet {
public:
    CompartmentSet(PillDispenser* dispenserA, PillDispenser* dispenserB, RotaryFunnel* funnel);
    void begin();

    void dispense(int compartmentNumber, int amount);

    void moveFunnelToPosition(RotaryFunnel::FunnelPosition position);

private:
    enum class CommandType { DISPENSE, MOVE_FUNNEL };

    struct Command {
        CommandType type;
        int compartmentNumber;
        int amount;
        RotaryFunnel::FunnelPosition funnelPosition;
    };

    PillDispenser* dispenserA;
    PillDispenser* dispenserB;
    RotaryFunnel* funnel;

    QueueHandle_t commandQueue = nullptr;

    void dispenseFromCompartment(PillDispenser* dispenser, int compartmentNumber, int amount);
    void processCommand(const Command& command);
    static void motorTask(void* param);
};
