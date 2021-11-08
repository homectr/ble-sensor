#include <avr/wdt.h>
#include "Device.h"

//#define NODEBUG_PRINT

Device* device;

void setup() {
    #ifndef NODEBUG_PRINT
    Serial.begin(115200);
    while (!Serial) { }
    Serial.println("RF24 sensor");
    #endif
    delay(1000); 
    device = new Device();
}

// watchdog interrupt
ISR(WDT_vect){
    wdt_disable();  // disable watchdog
}

void loop() {
    device->loop();
}
