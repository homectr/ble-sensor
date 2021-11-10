#pragma once

#include <Arduino.h>
#include <RF24.h>
#include "Input.h"
#include "RFPacket.h"
#include "Item.h"
#include "List.h"
#include "Indicator.h"

#define EXTERNAL_INTERRUPT_PIN  2

#define DHT11_PIN       4

#define CONTACT_PIN     5

#define BUTTON_PIN      6

#define LED_PIN_RED     7
#define LED_PIN_GREEN   8

#define NRF_CE_PIN      9
#define NRF_CSN_PIN     10

#define NRF_CHANNEL     120

#define RF24BR_BRIDGE_ADDRESS       0xB1B2B3B401LL
#define RF24BR_ACTUATOR_ADDRESS     0xA1A2A3A4A5LL



#define DEEP_SLEEP_INTERVAL_MULTIPLE    1

class Device {
    protected:
        bool isConfigMode = false;
        uint16_t deviceId;
        unsigned long aliveTimer = 0;

        InputBinary button = InputBinary(BUTTON_PIN,true);
        LEDIndicator indicator = LEDIndicator(LED_PIN_GREEN, LED_PIN_RED);

        RF24 radio = RF24(NRF_CE_PIN,NRF_CSN_PIN);
        RFSensorPacket buffer;

        List<Sensor> sensors = List<Sensor>();
        

    protected: 
        uint16_t getDeviceId();
        void sendBuffer();
        void normalMode();
        void configMode();
        
    public:
        Device();
        void loop();
};
