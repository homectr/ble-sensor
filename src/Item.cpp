#include "Item.h"

Item::Item(uint16_t itemId){
    this->itemId = itemId;
}

Sensor::Sensor(uint16_t itemId):Item(itemId){
    // empty for now
}

// fill in sensor sub-address and sensor type
void Sensor::initPacket(RFSensorPacket& packet){
    packet.srcAdr &= 0xFFFF0000;
    packet.srcAdr |= this->itemId;
    packet.sensorType = this->itemType;
}

void Sensor::announce(RFSensorPacket& packet){
    initPacket(packet);
}

SensorTempDHT21::SensorTempDHT21(uint16_t itemId, uint8_t pin):Sensor(itemId){
    this->pin = pin;
    this->itemType = RFSensorType::TEMPERATURE;
}

void SensorTempDHT21::read(RFSensorPacket& packet){
    initPacket(packet);
}

