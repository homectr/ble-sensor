#pragma once

#include "RFPacket.h"

class Item {

    protected: 
        uint16_t itemId;
        RFSensorType itemType;

    public:
        Item(uint16_t itemId);
        virtual void announce(RFSensorPacket& packet)=0;

};

class Sensor: public Item {

    protected:
        void initPacket(RFSensorPacket& packet);

    public:
        Sensor(uint16_t itemId);
        virtual void read(RFSensorPacket& packet)=0;
        void announce(RFSensorPacket& packet) override;
    
};

class SensorTempDHT21: public Sensor {
    protected:
        uint8_t pin;

    public:
        SensorTempDHT21(uint16_t itemId, uint8_t pin);
        void read(RFSensorPacket& packet) override;
};
