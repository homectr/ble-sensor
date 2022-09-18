#pragma once

#include <Arduino.h>

enum IndicatorMode : uint8_t
{
    UNDEFINED,
    NORMAL,
    CONFIG,
    IDENTIFICATION
};

class LEDIndicator
{
protected:
    uint8_t greenPin;
    uint8_t redPin;
    bool useGreen = false;
    bool useRed = false;

    bool isError = false;
    IndicatorMode mode = IndicatorMode::UNDEFINED;
    IndicatorMode previousMode = IndicatorMode::UNDEFINED;
    uint32_t modeTimer = 0;
    uint32_t modeTimeout;
    uint32_t ledOffPeriod = 0;
    uint32_t ledTimer = 0;
    uint32_t ledTimeout = 0;
    bool isOn = false;

public:
    LEDIndicator(uint8_t greenPin, uint8_t redPin);
    void setError(bool error) { this->isError = error; };
    void setMode(IndicatorMode mode, uint32_t timeout = 0);
    // blocking one-time blink
    void blink();
    // non-blocking blinking
    void loop();
    inline void on();
    inline void off();
};