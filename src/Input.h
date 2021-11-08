#pragma once

#include <Arduino.h>

class InputBinary {

    unsigned char pin;
    bool inverse = false;
    unsigned long debounceTimer = 0;
    unsigned long debounceTimeout = 100;
    bool debounceTimerOn = false;
    bool _isOpen = true;

    public:
        InputBinary(unsigned char pin, bool inverse = false);
        bool isOpen();
        void loop();
};