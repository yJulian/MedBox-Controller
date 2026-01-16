#include "compartment_set.hpp"
#include <esp32-hal.h>
#define dispense_intra_delay_ms 500

CompartmentSet::CompartmentSet(PillDispenser* dispenserA, PillDispenser* dispenserB, RotaryFunnel* funnel)
    : dispenserA(dispenserA), dispenserB(dispenserB), funnel(funnel) {
}

void CompartmentSet::begin() {
    // Initialize dispensers if they have begin methods
    if (dispenserA) {
        dispenserA->begin();
    }
    if (dispenserB) {
        dispenserB->begin();
    }
    if (funnel) {
        funnel->begin();
    }
}

void CompartmentSet::moveFunnelToPosition(RotaryFunnel::FunnelPosition position) {
    if (funnel) {
        funnel->rotateToPosition(position);
    }
}

void CompartmentSet::dispense(int compartmentNumber, int amount) {
    if ((compartmentNumber == 0 || compartmentNumber == 1) && dispenserA) {
        dispenseFromCompartment(dispenserA, amount);
    } else if (dispenserB) {
        dispenseFromCompartment(dispenserB, amount);
    } 
}

void CompartmentSet::dispenseFromCompartment(PillDispenser* dispenser, int compartmentNumber, int amount) {
    for (int i = 0; i < amount; ++i) {
        if (compartmentNumber % 2 == 0) {
            dispenser->dispensePillCompartmentA();
        } else {
            dispenser->dispensePillCompartmentB();
        }
        vTaskDelay(dispense_intra_delay_ms / portTICK_PERIOD_MS); // Delay between dispensing pills
    }
}