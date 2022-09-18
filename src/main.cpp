#include <avr/wdt.h>
#include "Device.h"

Device *device;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
    }
    Serial.println("\n*** Starting RF24 sensor");
    delay(1000); // delay for user to be able to push button after power-up to start config mode
    device = new Device();
}

void loop()
{
    device->loop();
}
