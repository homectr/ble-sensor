#pragma once

#include "RFPacket.h"

class Item {

    protected: 
        RFSensorPacket packet;
        uint16_t deviceId;
        uint16_t itemId;
        RFSensorType itemType;

    public:
        Item(uint16_t deviceId, uint16_t itemId);

};

class Sensor: public Item {

    public:
        Sensor(uint16_t deviceId, uint16_t itemId);
        virtual void read(RFSensorPacket& packet)=0;
    
};

class SensorTempDHT21: public Sensor {
    protected:
        uint8_t pin;

    public:
        SensorTempDHT21(uint16_t deviceId, uint16_t itemId, uint8_t pin);
        void read(RFSensorPacket& packet) override;
};
