#ifndef NETWORK_hh
#define NETWORK_hh

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "motors.hh"
//#define DEBUG

class Network
{
    public:
        enum PACKET_DATA_TYPE: char{NONE = ' ', ANGLE = 'A', LATENCY = 'P', STRING = 'S'};

        Network(Motors* motors);
        void initNetwork(const char *s, const char *pwd, IPAddress& ip,
                        IPAddress& gateway, IPAddress& subnet);

        void initReceiver(const uint16_t& port);
        void initTransmitter(const IPAddress& controllerIp, const uint16_t& port);
        void closeTransmitter();
        void sendBatterVoltage(const uint16_t& voltage);
        void timerCloseCheck();
        bool connected();
        bool packetHandler();
        PACKET_DATA_TYPE getPacketDataType();

    private:
        bool checkCounter(uint8_t *data, const uint8_t &len);
        void sendString(const String& data);
        void sendChar(const char character);
        void sendBuffer(uint8_t *buffer, uint8_t len);
        void wifiEventHandler(WiFiEvent_t event);

        enum PACKET_HEADER_TYPE: char{RESET = 'R', COUNTER = 'C', START = 'S', DISCONNECT = 'D'};
        
        WiFiUDP* udp_;
        Motors* motors_;
        uint32_t packetCounter_;
        bool connected_;
        IPAddress controllerIp_;
        uint16_t controllerPort_;
        uint8_t connectionCounter_;
        PACKET_DATA_TYPE packetDataType_;
};

#endif