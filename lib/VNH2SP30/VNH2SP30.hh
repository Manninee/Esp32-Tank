#ifndef VNH2SP30_hh
#define VNH2SP30_hh

#include <Arduino.h>

// Source: https://www.st.com/resource/en/datasheet/cd00043711.pdf

class VNH2SP30
{
public:
    VNH2SP30(uint8_t en, uint8_t pwm, uint8_t inA, uint8_t inB);
    void setUpPwm(uint8_t resolution, uint16_t freq, uint8_t channel);

    void stop();
    bool setSpeed(uint16_t value);
    bool setSpeedPercentage(uint8_t dutyCycle);
    void reverse();
    void forward();
    void enable();
    void disable();

private:
    uint8_t enable_;
    uint8_t pwm_;
    uint8_t inA_;
    uint8_t inB_;
    uint8_t pwmChannel_;
    uint16_t maxPwmValue_;
};


#endif