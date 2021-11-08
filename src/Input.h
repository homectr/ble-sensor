#pragma once

#include <Arduino.h>

class InputBinary {

    unsigned char pin;
    bool inverse = false;
    unsigned long debounceTimer = 0;
    unsigned long debounceTimeout = 100;
    bool debounceTimerOn = false;
    bool _isOpen = true;
    unsigned long stateStart; // when was input opened/closed

    public:
        InputBinary(unsigned char pin, bool pullup = false);
        bool isOpen();
        void loop();
        unsigned long stateDuration(){return millis()-stateStart;};
};