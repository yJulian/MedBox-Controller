#pragma once

#include "pill_dispenser.hpp"
#include "rotary_funnel.hpp"

class CompartmentSet {
public:
    CompartmentSet(PillDispenser* dispenserA, PillDispenser* dispenserB, RotaryFunnel* funnel);
    void begin();

    void dispense(int compartmentNumber, int amount);

    void moveFunnelToPosition(RotaryFunnel::FunnelPosition position);

private:
    PillDispenser* dispenserA;
    PillDispenser* dispenserB;
    RotaryFunnel* funnel;

    void dispenseFromCompartment(PillDispenser* dispenser, int amount);
};
