#include "Sensor.h"

Item::Item(uint16_t deviceId, uint16_t itemId){
    this->deviceId = deviceId;
    this->itemId = itemId;
}

Sensor::Sensor(uint16_t deviceId, uint16_t itemId):Item(deviceId, itemId){
    // empty for now
}

SensorTempDHT21::SensorTempDHT21(uint16_t deviceId, uint16_t itemId, uint8_t pin):Sensor(deviceId, itemId){
    this->pin = pin;
    this->itemType = RFSensorType::TEMPERATURE;
}

void SensorTempDHT21::read(RFSensorPacket& packet){
    packet.srcAdr &= 0xFFFF0000;
    packet.srcAdr |= this->itemId;
    packet.itemType = this->itemType;

}