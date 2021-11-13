#pragma once

#include <Arduino.h>

// generate 16bit UUID
uint16_t generate_id16();

// calculate CRC32 of give buffer
uint32_t crc32(void* buff, size_t length);

// Arduino VCC voltage in mV
uint16_t readVCC(); 

// restarts Arduino
void reboot();