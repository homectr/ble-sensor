#include "Indicator.h"

#define LED_ON_PERIOD    200

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
    if (isError) digitalWrite(redPin,1);
    else digitalWrite(greenPin,1);
    isOn=true;
}

void LEDIndicator::off(){
    digitalWrite(greenPin,0);
    digitalWrite(redPin,0);
    isOn=false;
}

void LEDIndicator::setMode(IndicatorMode mode){
    this->mode = mode;
    switch (this->mode) {
    case IndicatorMode::NORMAL:
        ledOffPeriod = 2000;
        
        break;
    case IndicatorMode::CONFIG:
        ledOffPeriod = 1000;
        break;
    case IndicatorMode::PAIRING:
        ledOffPeriod = 300;
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
    if (millis()-ledTimer > ledTimeout){
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