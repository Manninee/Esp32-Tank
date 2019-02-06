#ifndef MOTORS_HH
#define MOTORS_HH

#include <Arduino.h>
#include "VNH2SP30.hh"

class Motors
{
    public:
        Motors(VNH2SP30* rightMotor, VNH2SP30* leftMotor);
        void enable();
        void disable();
        void setStatus(const int16_t& x, const int16_t& y);
        bool getReadBatteryVoltage();

    private:
        VNH2SP30* rightMotor_;
        VNH2SP30* leftMotor_;
        bool readBatteryVoltage_;
};
#endif