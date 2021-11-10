#pragma once

#include <Arduino.h>

enum IndicatorMode: uint8_t {
    NORMAL,
    CONFIG,
    IDENTIFICATION
};

class LEDIndicator {
    protected:
    uint8_t greenPin;
    uint8_t redPin;
    bool useGreen = false;
    bool useRed = false;

    bool isError = false;
    IndicatorMode mode = IndicatorMode::NORMAL;
    IndicatorMode previousMode = IndicatorMode::NORMAL;
    uint32_t modeTimer;
    uint32_t modeTimeout;
    uint32_t ledOffPeriod;
    uint32_t ledTimer;
    uint32_t ledTimeout;
    bool isOn = false;

    public:
    LEDIndicator(uint8_t greenPin, uint8_t redPin);
    void setError(bool error){this->isError = error;};
    void setMode(IndicatorMode mode, uint32_t timeout = 0);
    // blocking one-time blink
    void blink();
    // non-blocking blinking
    void loop();
    inline void on();
    inline void off();

};