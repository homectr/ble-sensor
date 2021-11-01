#include <SPI.h>
#include <RF24.h>
#include <BTLE.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#define NRF_CE_PIN 9
#define NRF_CSN_PIN 10

RF24 radio(NRF_CE_PIN,NRF_CSN_PIN);
BTLE btle(&radio);

char deviceSignature[7] = "HCTR--";
char deviceType = 'A';
char deviceNum = 'A';

void setup() {
    Serial.begin(9600);
    while (!Serial) { }
    Serial.println("BTLE advertisement sender");
    deviceSignature[4] = deviceType;
    deviceSignature[5] = deviceNum;
    btle.begin(deviceSignature);
}

// watchdog interrupt
ISR(WDT_vect){
    wdt_disable();  // disable watchdog
}

void sleepNow(const byte interval) {  
    Serial.println("going to sleep");
    digitalWrite(13, LOW);                 // turn off LED13 status on-board LED
    unsigned char spi_save = SPCR;
    SPCR = 0;                // disable SPI
    power_adc_disable();
    power_spi_disable();
    power_timer0_disable();
    power_timer1_disable();
    power_timer2_disable();
    power_twi_disable(); 

    MCUSR = 0;                          // reset various flags
    WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
    WDTCSR =  0b01000000 | interval;    // set WDIE, and appropriate delay

    wdt_reset();
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
    sleep_mode();            // now goes to Sleep and waits for the interrupt

    sleep_disable();         // first thing after waking from sleep: disable sleep...    
    detachInterrupt(1);      // disables interrupt 1 on pin 3 so the 
                             // wakeUpNow code will not be executed 
                             // during normal running time.
  
    power_all_enable();
    SPCR = spi_save;            // restore SPI
}

int i = 0;
float temp=30;
nrf_service_data buf;
char buf2[8] = "HCTRBK";

void loop() {  
    radio.powerUp();
    bool a = btle.advertise(0x16, &buf2, sizeof(buf2));
    delay(1000);
    a = btle.advertise(0x16, &buf2, sizeof(buf2));
    
    btle.hopChannel();
    btle.hopChannel();
    if (!a) Serial.print("e");
    else Serial.print(".");
    delay(1000);
    i++;
    if (i>50){
        Serial.println();
        i=0;
    }
    radio.powerDown();

    // sleep for a total of Nx8 seconds
    for (char i=0;i<2;i++)
        sleepNow(0b100001);  // 8 seconds
    
}