#include "pill_dispenser_stepper.hpp"
#include "stepper/Stepper.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


class PillDispenserEncoder: public PillDispenserStepper {
public:
    PillDispenserEncoder(int number_of_steps, int m1, int m2, int m3, int m4, uint8_t control_switch);

    void dispensePillCompartmentA();
    void dispensePillCompartmentB();

    void begin();

private:
    uint8_t control_switch;
    bool calibrate();
};