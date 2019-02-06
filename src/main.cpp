#include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>
#include <driver/adc.h>

#define DEBUG

#include "network.hh"
#include "VNH2SP30.hh"
#include "motors.hh"

#define PORT 8080

void handleBatteryVoltage();

// Enable, PWM, InA, InB
VNH2SP30 rightMotor(26, 12, 27, 14);
VNH2SP30 leftMotor(5, 21, 19, 18);
Motors motors(&rightMotor, &leftMotor);
Network network(&motors);

const char * SSID = "Tank";
const char * PWD = "Something";
IPAddress IP(192, 168, 1, 250);
IPAddress SUBNET(255, 255, 255, 0);

Ticker connectionTimer;
Ticker secondTimer;

volatile bool checkConnectionTime = false;
volatile bool secondPassed = false;

void setup()
{
  #ifdef DEBUG
    Serial.begin(115200);
    delay(3000);
  #endif

  network.initNetwork(SSID, PWD, IP, IP, SUBNET);
  network.initReceiver(PORT);
  
  connectionTimer.attach_ms(200, [](){checkConnectionTime = true;});
  secondTimer.attach(1, [](){secondPassed = true;});

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_6db);
}

void loop()
{
  network.packetHandler();
  
  // Check for connection every 200ms
  if(checkConnectionTime)
  {
    network.timerCloseCheck();
    checkConnectionTime = false;
  }

  // Check battery voltage every second
  if(secondPassed)
  {
    handleBatteryVoltage();
    secondPassed = false;
  }
}

void handleBatteryVoltage()
{
  if(motors.getReadBatteryVoltage())
  {
    uint16_t voltage = adc1_get_raw(ADC1_CHANNEL_6);
    voltage /= 2.76; // Measured constant
    network.sendBatterVoltage(voltage); 
  }
}