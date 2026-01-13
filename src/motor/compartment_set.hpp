#include "pill_dispenser.hpp"

class CompartmentSet {
public:
    CompartmentSet(PillDispenser* dispenserA, PillDispenser* dispenserB);
    void begin();

    void dispense(int compartmentNumber, int amount);

private:
    PillDispenser* dispenserA;
    PillDispenser* dispenserB;

    void dispenseFromCompartment(PillDispenser* dispenser, int amount);
};
