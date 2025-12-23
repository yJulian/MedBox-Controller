#pragma once

class Motor {
    public:
        Motor();
        void initialize();
        void setSpeed(double speed);
        void step(int steps);
        void stop();
};