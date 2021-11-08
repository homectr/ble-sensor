#include "Input.h"

//#define NODEBUG_PRINT

InputBinary::InputBinary(unsigned char pin, bool pullup){
    this->pin = pin;
    this->inverse = pullup;
    if (pullup)
        pinMode(pin,INPUT_PULLUP);
    else
        pinMode(pin,INPUT);
}

bool InputBinary::isOpen(){
    return _isOpen;
}

void InputBinary::loop(){
    if (debounceTimerOn && millis()-debounceTimer < debounceTimeout) return;
    bool _p = digitalRead(pin);
    if (_isOpen != _p){
        debounceTimerOn = true;
        debounceTimer = millis();
        stateStart = millis();
        if (inverse) 
            _isOpen = _p;
        else
            _isOpen = !_p;
        #ifndef NODEBUG_PRINT
        Serial.print("[Input] ms= ");
        Serial.print(millis());
        Serial.print(" pin=");
        Serial.print(pin);
        Serial.print(" inv=");
        Serial.print(inverse);
        Serial.print(" status=");
        Serial.println(_isOpen?"OPEN":"CLOSED");
        #endif
    }

}