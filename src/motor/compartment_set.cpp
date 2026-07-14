#include "compartment_set.hpp"
#include <esp32-hal.h>
#define dispense_intra_delay_ms 500

// BLE/WiFi protocol stack tasks run pinned to core 0, so the motor task is
// pinned to core 1 to keep long, blocking stepper moves from stalling them.
#define MOTOR_TASK_CORE 1
#define MOTOR_TASK_STACK_SIZE 4096
#define MOTOR_TASK_PRIORITY 1
#define MOTOR_QUEUE_LENGTH 10

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

    commandQueue = xQueueCreate(MOTOR_QUEUE_LENGTH, sizeof(Command));

    xTaskCreatePinnedToCore(
        motorTask,
        "motorTask",
        MOTOR_TASK_STACK_SIZE,
        this,
        MOTOR_TASK_PRIORITY,
        NULL,
        MOTOR_TASK_CORE
    );
}

void CompartmentSet::motorTask(void* param) {
    CompartmentSet* self = static_cast<CompartmentSet*>(param);
    Command command;
    for (;;) {
        if (xQueueReceive(self->commandQueue, &command, portMAX_DELAY) == pdTRUE) {
            self->processCommand(command);
        }
    }
}

void CompartmentSet::processCommand(const Command& command) {
    if (command.type == CommandType::DISPENSE) {
        if ((command.compartmentNumber == 0 || command.compartmentNumber == 1) && dispenserA) {
            dispenseFromCompartment(dispenserA, command.compartmentNumber, command.amount);
        } else if (dispenserB) {
            dispenseFromCompartment(dispenserB, command.compartmentNumber, command.amount);
        }
    } else if (funnel) {
        funnel->rotateToPosition(command.funnelPosition);
    }
}

void CompartmentSet::moveFunnelToPosition(RotaryFunnel::FunnelPosition position) {
    Command command{CommandType::MOVE_FUNNEL, 0, 0, position};
    xQueueSend(commandQueue, &command, portMAX_DELAY);
}

void CompartmentSet::dispense(int compartmentNumber, int amount) {
    Command command{CommandType::DISPENSE, compartmentNumber, amount, RotaryFunnel::POSITION_0};
    xQueueSend(commandQueue, &command, portMAX_DELAY);
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