#include "VNH2SP30.hh"

VNH2SP30::VNH2SP30(uint8_t en, uint8_t pwm, uint8_t inA, uint8_t inB):
enable_(en), pwm_(pwm), inA_(inA), inB_(inB), pwmChannel_(0), maxPwmValue_(0)
{
    pinMode(enable_, OUTPUT);
    pinMode(pwm_, OUTPUT);
    pinMode(inA_, OUTPUT);
    pinMode(inB_, OUTPUT);

    digitalWrite(inA_, LOW);
    digitalWrite(inB_, LOW);
    disable();
}

void VNH2SP30::setUpPwm(uint8_t resolution, uint16_t freq, uint8_t channel)
{
    pwmChannel_ = channel;
    maxPwmValue_ = pow(2, resolution) - 1;
    ledcSetup(pwmChannel_, freq, resolution);
    ledcAttachPin(pwm_, pwmChannel_);
    ledcWrite(pwmChannel_, 0);
}

void VNH2SP30::stop()
{
    digitalWrite(inA_, LOW);
    digitalWrite(inB_, LOW);
    ledcWrite(pwmChannel_, 0);
}
bool VNH2SP30::setSpeed(uint16_t value)
{
    if(value > maxPwmValue_) return 0;
    ledcWrite(pwmChannel_, value);
    return 1;
}

bool VNH2SP30::setSpeedPercentage(uint8_t dutyCycle)
{
    if(dutyCycle < 100) return 0;
    uint16_t newPwmValue = (dutyCycle / 100) * maxPwmValue_;
    ledcWrite(pwmChannel_, newPwmValue);
    return 1;
}

void VNH2SP30::reverse()
{
    digitalWrite(inA_, LOW);
    digitalWrite(inB_, HIGH);
}

void VNH2SP30::forward()
{
    digitalWrite(inA_, HIGH);
    digitalWrite(inB_, LOW);
}

void VNH2SP30::enable()
{
    digitalWrite(enable_, HIGH);
}

void VNH2SP30::disable()
{
    digitalWrite(enable_, LOW);
}