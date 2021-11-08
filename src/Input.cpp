#include "Input.h"

//#define NODEBUG_PRINT

InputBinary::InputBinary(unsigned char pin, bool inverse){
    this->pin = pin;
    this->inverse = inverse;
    pinMode(pin,INPUT_PULLUP);
}

bool InputBinary::isOpen(){
    return _isOpen;
}

void InputBinary::loop(){
    if (debounceTimerOn && millis()-debounceTimer < debounceTimeout) return;
    if (_isOpen != digitalRead(pin)){
        debounceTimerOn = true;
        debounceTimer = millis();
        if (inverse)
            _isOpen = digitalRead(pin);
        else
            _isOpen = !digitalRead(pin);
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