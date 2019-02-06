#include "motors.hh"

Motors::Motors(VNH2SP30* rightMotor, VNH2SP30* leftMotor):
rightMotor_(rightMotor), leftMotor_(leftMotor),
readBatteryVoltage_(true)
{
    rightMotor_ = rightMotor;
    leftMotor_ = leftMotor;
    rightMotor_->setUpPwm(10, 10000, 0);
    leftMotor_->setUpPwm(10, 10000, 1);
}

void Motors::enable()
{
    rightMotor_->enable();
    leftMotor_->enable();
}

void Motors::disable()
{
    rightMotor_->setSpeed(0);
    leftMotor_->setSpeed(0);

    rightMotor_->stop();
    leftMotor_->stop();
    
    rightMotor_->disable();
    leftMotor_->disable();

    readBatteryVoltage_ = true;
}

void Motors::setStatus(const int16_t& x, const int16_t& y)
{
    // If motors not running enable battery voltage reading
    if(x == 0 && y == 0)
    {
        readBatteryVoltage_ = true;
        rightMotor_->stop();
        leftMotor_->stop();
        return;
    }

    readBatteryVoltage_ = false;

    // Calculate values for the motors. Algorithm source http://home.kendra.com/mauser/joystick.html
    int16_t invx = -x;
    double V = ((1023 - abs(invx)) * (y / 1.023)) /1000 + y;
    double W = ((1023 - abs(y)) * (invx / 1.023)) /1000 + invx;
    int16_t right = round((V + W) / 2);
    int16_t left = round((V - W) / 2);

    #ifdef DEBUG
        Serial.print("right="); Serial.print(right);
        Serial.print(" left="); Serial.print(left);
        Serial.println();
    #endif

    // Set rotation direction for the motors
    if(right < 0) 
        rightMotor_->reverse();
    else
        rightMotor_->forward();

    if(left < 0)
        leftMotor_->reverse();
    else
        leftMotor_->forward();

    rightMotor_->setSpeed(abs(right));
    leftMotor_->setSpeed(abs(left));
}

bool Motors::getReadBatteryVoltage()
{
    return readBatteryVoltage_;
}