#include "network.hh"

Network::Network(Motors* motors):
udp_(new WiFiUDP),
motors_(motors),
packetCounter_(1),
connected_(false),
controllerIp_(IPAddress(0,0,0,0)),
controllerPort_(0),
connectionCounter_(0),
packetDataType_(NONE)
{}

void Network::initNetwork(const char *s, const char *pwd, IPAddress& ip,
                            IPAddress& gateway, IPAddress& subnet)
{
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ip, gateway, subnet);
    auto f = std::bind(&Network::wifiEventHandler, this, std::placeholders::_1);
    WiFi.onEvent(f);
    WiFi.softAP(s, pwd);

    #ifdef DEBUG
        Serial.println("Access point ready");
        Serial.println(WiFi.softAPIP());
    #endif
}

void Network::initReceiver(const uint16_t& port)
{
    /*
    udp_->listen(port);
    // set packetHandler method to handle packets
    auto f = std::bind(&Network::packetHandler, this, std::placeholders::_1);
    udp_->onPacket(f);
    */
    
    udp_->begin(port);

    #ifdef DEBUG
        Serial.println("Receiver ready");
    #endif
}

void Network::initTransmitter(const IPAddress& controllerIp, const uint16_t& port)
{
    controllerIp_ = controllerIp;
    controllerPort_ = port;
    packetCounter_ = 1;
    connectionCounter_ = 0;
    connected_ = true;
    motors_->enable();

    #ifdef DEBUG
        Serial.println("Controller connected");
        Serial.print("Controller at ");
        Serial.println(controllerIp_);
    #endif
}

void Network::closeTransmitter()
{
    connected_ = false;
    controllerIp_ = IPAddress(0,0,0,0);
    connectionCounter_ = 0;
    controllerPort_ = 0;
    packetCounter_ = 1;

    motors_->disable();

    #ifdef DEBUG
        Serial.println("Connection closed");
    #endif
}

void Network::sendBatterVoltage(const uint16_t& voltage)
{
    if(!connected_)
        return;
    
    uint8_t buffer[3] = {'B',
                        (uint8_t)(voltage >> 8),
                        (uint8_t)voltage};
    sendBuffer(buffer, 3);

    #ifdef DEBUG
        Serial.print("Battery voltage=");
        Serial.println(voltage);
    #endif
}

void Network::timerCloseCheck()
{
    if(!connected_) return;

    ++connectionCounter_;
    #ifdef DEBUG
        Serial.print("Counter");
        Serial.println(connectionCounter_);
    #endif

    if(connectionCounter_ > 2)
        closeTransmitter();
}

bool Network::connected()
{
    return connected_;
}

bool Network::packetHandler()
{
    uint8_t packetSize = udp_->parsePacket();
    if(!packetSize)
        return 0;

    uint8_t data[packetSize];
    udp_->read(data, packetSize);

    if(checkCounter(data, packetSize) || packetSize < 6)
        return 0;

    #ifdef DEBUG
        Serial.print("data type ="); Serial.println((char)data[5]);
    #endif

    packetDataType_ = NONE;

    switch (data[5])
    {
    case ANGLE:
    {
        int16_t x = (int16_t)(data[6] << 8);
        x |= (int16_t)data[7];
        x -= 1023;
        
        int16_t y = (int16_t)(data[8] << 8);
        y |= (int16_t)data[9];
        y -= 1023;

        motors_->setStatus(x, y);

        packetDataType_ = ANGLE;

        #ifdef DEBUG
            Serial.print("x="); Serial.print(x);
            Serial.print("y="); Serial.print(y);
            Serial.println();
        #endif
        break;
    }

    case LATENCY:
        sendChar('P');
        packetDataType_ = LATENCY;
        #ifdef DEBUG
            Serial.println("Latency check");
        #endif
        break;

    case STRING:
    {
        String msg = "";
        for(uint8_t i = 6; i < packetSize; ++i)
            msg += (char)data[i];
        packetDataType_ = STRING;
        break;
    } 
    default:
        return 0;
    }
    return 1;
}

Network::PACKET_DATA_TYPE Network::getPacketDataType()
{
    return packetDataType_;
}

bool Network::checkCounter(uint8_t *data, const uint8_t &len)
{
    #ifdef DEBUG
        Serial.println("-----------------------");
        Serial.print(udp_->remoteIP());
        Serial.print(" Command=");
        Serial.println((char)data[0]);
    #endif

    if(data[0] != START)
        if(udp_->remoteIP() != controllerIp_) return 1;

    connectionCounter_ = 0;

    switch (data[0])
    {
    case RESET:
        // Reset packet counter
        packetCounter_ = 1;
        break;

    case COUNTER:
    {
        // Calculate packet number
        if(len < 5) return 1;
        
        uint32_t packetNumber = 0;
        for(uint8_t i = 1; i < 5; ++i)
        {
            uint8_t shiftCount = 8 *(4 - i);
            packetNumber |= (uint32_t)(data[i] << shiftCount);
        }

        #ifdef DEBUG
            Serial.print("Counter="); Serial.print(packetCounter_);
            Serial.print(" Number="); Serial.println(packetNumber);
        #endif

        if(packetNumber > packetCounter_)
        {
            packetCounter_ = packetNumber;
            return 0;
        }
        break;
    }
    
    case START:
    {
        if(len < 3 || connected_) return 1;

        uint16_t port = (uint16_t)(data[1] << 8);
        port |= (uint16_t)data[2];
        
        // Start transmitter
        initTransmitter(udp_->remoteIP(), port);
        break;
    }

    case DISCONNECT:
        // Stop listening controller
        if(!connected_)
            return 1;
        closeTransmitter();
        break;

    default:
        break;    
    }
    return 1;
}

void Network::sendString(const String& data)
{
    if(!connected_) 
        return;

    uint16_t len = data.length();
    uint8_t buffer[len];
    data.getBytes(buffer, len);
    sendBuffer(buffer, len);
}

void Network::sendChar(const char character)
{
    udp_->beginPacket(controllerIp_, controllerPort_);
    udp_->write(character);
    udp_->endPacket();
}

void Network::sendBuffer(uint8_t *buffer, uint8_t len)
{
    udp_->beginPacket(controllerIp_, controllerPort_);
    udp_->write(buffer, len);
    udp_->endPacket();
}

void Network::wifiEventHandler(WiFiEvent_t event)
{
    switch (event)
    {
        case SYSTEM_EVENT_STA_CONNECTED:
            break;
    
        case SYSTEM_EVENT_STA_DISCONNECTED:
            closeTransmitter();
            break;
        default:
            break;
    }
}