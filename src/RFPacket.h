#pragma once

#include <Arduino.h>

enum RFSensorType: uint8_t {
    TEMPERATURE,
    CONTACT,
    HUMIDITY
};

enum RFPacketType: uint8_t {
    SCAN,
    DATA,
    IDENTIFY,
    ANNOUNCE
};

// when changing RFPacket structure be aware of potential padding bytes
// as it does not have to be the same on all platforms
using RFSensorPayload = uint8_t[21];
struct RFSensorPacket {
    uint32_t seqno;
    uint32_t srcAdr;
    RFPacketType pktType;
    RFSensorType sensorType;
    uint8_t vcc;
    RFSensorPayload payload;
};

using RFActuatorPayload = uint8_t[23];
struct RFActuatorPacket {
    uint32_t seqno;
    uint32_t dstAdr;
    RFPacketType pktType;
    RFActuatorPayload payload;
};