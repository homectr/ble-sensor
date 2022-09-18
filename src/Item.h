#pragma once

#include "RFPacket.h"
#include <DHT_U.h>

class Item
{

protected:
    uint16_t itemId;
    RFSensorType itemType;
    uint8_t reqInitTime = 0;

public:
    Item(uint16_t itemId);
    virtual void announce(RFSensorPacket &packet) = 0;
    uint8_t requiresInitTime() { return reqInitTime; };
    uint16_t getId() { return itemId; };
    RFSensorType getType() { return itemType; };
};

class Sensor : public Item
{

protected:
    void initPacket(RFSensorPacket &packet);

public:
    Sensor(uint16_t itemId);
    virtual void read(RFSensorPacket &packet) = 0;
    void announce(RFSensorPacket &packet) override;
};

class SensorDHTTemp : public Sensor
{
protected:
    DHT_Unified *dht;

public:
    SensorDHTTemp(uint16_t itemId, DHT_Unified *dht);

    // read returns payload containing sensor value C-string
    // e.g. "28.5" - value is in degrees of Celsius
    void read(RFSensorPacket &packet) override;
};

class SensorDHTHumidity : public Sensor
{
protected:
    DHT_Unified *dht;

public:
    SensorDHTHumidity(uint16_t itemId, DHT_Unified *dht);

    // read returns payload containing sensor value C-string
    // e.g. "41.5" relative humidity
    void read(RFSensorPacket &packet) override;
};

class SensorContact : public Sensor
{

protected:
    uint8_t pin;

public:
    // Contact requires external interrupt as change can happen while sensor sleeps.
    // Make sure reflecting sensor change on INT0. E.g. by attaching your sensor in parralel to both pin you would like to use and to pin attached to INT0.
    // Please, check MCU documentation to see which pin is INT0. E.g. Arduino Mini Pro it is pin 2;
    SensorContact(uint16_t itemId, uint8_t pin);

    // read returns payload containing sensor value as C-string
    // "0" = open, "1" = closed
    virtual void read(RFSensorPacket &packet);
};
