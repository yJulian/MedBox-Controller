#pragma once

class PillDispenser {
public:
    PillDispenser();

    virtual void begin();

    virtual void dispensePillCompartmentA();
    virtual void dispensePillCompartmentB();
};