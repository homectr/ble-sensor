#include "Item.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>

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

SensorDHTTemp::SensorDHTTemp(uint16_t itemId, DHT_Unified *dht):Sensor(itemId){
    this->itemType = RFSensorType::TEMPERATURE;
    this->dht = dht;
}

void SensorDHTTemp::read(RFSensorPacket& packet){
    initPacket(packet);
    sensors_event_t event;
    String v = "NaN";
    dht->temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        #ifndef NODEBUG_PRINT
        Serial.println(F("Error reading temperature!"));
        #endif
    }
    else {
        #ifndef NODEBUG_PRINT
        Serial.print(F("  Temperature: "));
        Serial.print(event.temperature);
        Serial.println(F("°C"));
        #endif
        v = String(event.temperature);
    }
    strcpy((char*)packet.payload,v.c_str());
}

SensorDHTHumidity::SensorDHTHumidity(uint16_t itemId, DHT_Unified* dht):Sensor(itemId){
    this->itemType = RFSensorType::HUMIDITY;
    this->dht = dht;
}

void SensorDHTHumidity::read(RFSensorPacket& packet){
    initPacket(packet);
    sensors_event_t event;
    String v = "NaN";
    // Get humidity event and print its value.
    dht->humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
        #ifndef NODEBUG_PRINT
        Serial.println(F("Error reading humidity!"));
        #endif
    }
    else {
        #ifndef NODEBUG_PRINT
        Serial.print(F("  Humidity: "));
        Serial.print(event.relative_humidity);
        Serial.println(F("%"));
        #endif
        v = String(event.relative_humidity);
    }
    strcpy((char*)packet.payload,v.c_str());
}



SensorContact::SensorContact(uint16_t itemId, uint8_t pin):Sensor(itemId){
    this->pin = pin;
    pinMode(pin, INPUT_PULLUP);
}

void SensorContact::read(RFSensorPacket& packet){
    initPacket(packet);
    String v = String(!digitalRead(pin));
    #ifndef NODEBUG_PRINT
    Serial.print(F("  Contact: "));
    Serial.println(v);
    #endif

    strcpy((char*)packet.payload,v.c_str());
}
 