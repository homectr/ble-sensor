#pragma once

#include <Arduino.h>

enum SleepDuration: unsigned char {
    DUR_8s = 0b00100001,
    DUR_4s = 0b00100000,
    DUR_2s = 0b00000111,
    DUR_1s = 0b00000110
};

// generate 16bit UUID
uint16_t generate_id16();

// calculate CRC32 of give buffer
uint32_t crc32(void* buff, size_t length);

// Arduino VCC voltage in mV
uint16_t readVCC(); 

void sleep(SleepDuration sleepDur);