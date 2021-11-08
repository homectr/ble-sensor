#pragma once

#include "RFPacket.h"
#include <DHT_U.h>

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

class SensorDHTTemp: public Sensor {
    protected:
        DHT_Unified *dht;

    public:
        SensorDHTTemp(uint16_t itemId, DHT_Unified* dht);
        void read(RFSensorPacket& packet) override;
};

class SensorDHTHumidity: public Sensor {
    protected:
        DHT_Unified *dht;

    public:
        SensorDHTHumidity(uint16_t itemId, DHT_Unified* dht);
        void read(RFSensorPacket& packet) override;
};
