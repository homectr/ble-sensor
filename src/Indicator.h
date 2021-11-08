#pragma once

#include <Arduino.h>

enum IndicatorMode: uint8_t {
    NORMAL,
    CONFIG,
    PAIRING
};

class LEDIndicator {
    protected:
    uint8_t greenPin;
    uint8_t redPin;

    bool isError = false;
    IndicatorMode mode = IndicatorMode::NORMAL;
    uint32_t ledOffPeriod;
    uint32_t ledTimer;
    uint32_t ledTimeout;
    bool isOn = false;

    public:
    LEDIndicator(uint8_t greenPin, uint8_t redPin);
    void setError(bool error){this->isError = error;};
    void setMode(IndicatorMode mode);
    // blocking one-time blink
    void blink();
    // non-blocking blinking
    void loop();
    inline void on();
    inline void off();

};