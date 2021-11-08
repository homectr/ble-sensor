#pragma once

#include <Arduino.h>

enum RFSensorType: uint8_t {
    TEMPERATURE,
    CONTACT
};

enum RFPacketType: uint8_t {
    SCAN,
    DATA
};

// when changing RFPacket structure be aware of potential padding bytes
// as it does not have to be the same on all platforms
struct RFSensorPacket {
    uint32_t seqno;
    uint32_t srcAdr;
    RFSensorType sensorType;
    uint8_t vcc;
    uint8_t payload[22];
};

struct RFActuatorPacket {
    uint32_t seqno;
    uint32_t dstAdr;
    RFPacketType pktType;
    uint8_t payload[23];
};