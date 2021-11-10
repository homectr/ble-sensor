#include "Indicator.h"

#define LED_ON_PERIOD    200

#define NODEBUG_PRINT

LEDIndicator::LEDIndicator(uint8_t greenPin, uint8_t redPin){
    this->greenPin=greenPin; 
    this->redPin=redPin;
    pinMode(greenPin,OUTPUT);
    pinMode(redPin,OUTPUT);
    ledTimer = millis();
    setMode(IndicatorMode::NORMAL);
    off();
}

void LEDIndicator::on(){
    if (useRed || isError) digitalWrite(redPin,1);
    if (useGreen) digitalWrite(greenPin,1);
    isOn=true;
}

void LEDIndicator::off(){
    digitalWrite(greenPin,0);
    digitalWrite(redPin,0);
    isOn=false;
}

void LEDIndicator::setMode(IndicatorMode mode, uint32_t timeout){
    if (mode == this->mode) return;
    modeTimeout = timeout;
    if (timeout > 0) {
        previousMode = this->mode;
        modeTimer = millis();
        #ifndef NODEBUG_PRINT
        Serial.print("Mode timeout ");
        Serial.print(modeTimeout);
        Serial.print("Prev Mode ");
        Serial.print(previousMode);

        #endif
    }
    this->mode = mode;
    switch (this->mode) {
    case IndicatorMode::NORMAL:
        ledOffPeriod = 2000;
        useGreen = true;
        useRed = false;
        break;
    case IndicatorMode::CONFIG:
        ledOffPeriod = 700;
        useGreen = true;
        useRed = false;
        break;
    case IndicatorMode::IDENTIFICATION:
        ledOffPeriod = 100;
        useGreen = true;
        useRed = true;
        break;
    
    default:
        break;
    }
    ledTimeout = ledOffPeriod;
    off();
}

void LEDIndicator::blink(){
    on();
    delay(LED_ON_PERIOD);
    off();
}

void LEDIndicator::loop(){
    if (modeTimeout) {
        if (millis()-modeTimer > modeTimeout)
            setMode(previousMode);
    }
    if (ledTimeout && millis()-ledTimer > ledTimeout){
        ledTimer = millis();
        if (isOn) {
            off();
            ledTimeout = ledOffPeriod;
        } else {
            on();
            ledTimeout = LED_ON_PERIOD;
        }
    }
}