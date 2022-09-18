#pragma once

#include <Arduino.h>
#include <RF24.h>
#include "Input.h"
#include "RFPacket.h"
#include "Item.h"
#include "List.h"
#include "Indicator.h"

#define EXTERNAL_INTERRUPT0_PIN 2 // pin to interrupt sleep and immediate measurement without external peripherals
#define EXTERNAL_INTERRUPT1_PIN 3 // pin to interrupt sleep, powerup external peripherals, wait 8s and measure

#define PERIPH_POWER_PIN A0

#define DHT11_PIN 4

#define CONTACT_PIN 5

#define BUTTON_PIN 6

#define LED_PIN_RED 7
#define LED_PIN_GREEN 8

#define NRF_CE_PIN 9
#define NRF_CSN_PIN 10

#define NRF_CHANNEL 120

#define RF24BR_BRIDGE_ADDRESS 0xB1B2B3B401LL
#define RF24BR_ACTUATOR_ADDRESS 0xA1A2A3A4A5LL

#define SLEEP_DURATION_8s 0b00100001
#define SLEEP_DURATION_4s 0b00100000
#define SLEEP_DURATION_2s 0b00000111
#define SLEEP_DURATION_1s 0b00000110

#define SLEEP_CYCLE_DURATION SLEEP_DURATION_8s
#define SLEEP_CYCLES_COUNT 40 // sleep ((n-1) x SLEEP_CYCLE_DURATION + 4) seconds

class Device
{
protected:
    bool isConfigMode = false;
    uint16_t deviceId;
    unsigned long aliveTimer = 0;

    InputBinary button = InputBinary(BUTTON_PIN, true);
    LEDIndicator indicator = LEDIndicator(LED_PIN_GREEN, LED_PIN_RED);

    RF24 radio = RF24(NRF_CE_PIN, NRF_CSN_PIN);
    RFSensorPacket buffer;

    List<Sensor> sensors = List<Sensor>();

    bool _announce = false;

protected:
    uint16_t getDeviceId();
    void sendBuffer();
    void normalMode();
    void configMode();
    // disconnect external peripherals, e.g. DHT sensor, from power to save batteries
    void disconnectPeripherals();
    // connect external peripherals to power
    void connectPeripherals();
    // sleep (n-1)x8+4 seconds
    void sleep(uint16_t multiple);
    void announceDevices();

public:
    Device();
    void loop();
};
