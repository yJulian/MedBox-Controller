#include "pill_dispenser.hpp"

class PillDispenserStepper: public PillDispenser {
public:
    PillDispenserStepper(int stepPin, int dirPin, int enablePin);
    
    void dispensePillCompartmentA();
    void dispensePillCompartmentB();

    void begin();

private:
    int stepPin;
    int dirPin;
    int enablePin;

    void stepMotor(int steps, bool direction);
};