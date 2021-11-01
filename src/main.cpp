#include <SPI.h>
#include <RF24.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#define NRF_CE_PIN      9
#define NRF_CSN_PIN     10
#define NRF_CHANNEL     120
#define NRF_BROADCAST_ADDRESS  "rf24se"

//#define NODEBUG_PRINT

RF24 radio(NRF_CE_PIN,NRF_CSN_PIN);
unsigned char address[7] = NRF_BROADCAST_ADDRESS;

// when changing RFPacket structure be aware of potential padding bytes
// as it does not have to be the same on all platforms
struct RFPacket {
    uint32_t seqno;
    unsigned char deviceType;
    unsigned char deviceId;
    unsigned char deviceSubId;
    unsigned char payload[25];
};

RFPacket buffer;

void setup() {
    #ifndef NODEBUG_PRINT
    Serial.begin(115200);
    while (!Serial) { }
    Serial.println("RF24 sensor");
    delay(100); 
    #endif

    if (!radio.begin()){
        //TODO Handle error
    }
    radio.setAutoAck(false); // disable auto-ack from receivers
    radio.setChannel(NRF_CHANNEL); 
    radio.setCRCLength(RF24_CRC_16); // set CRC length
    radio.setPALevel(RF24_PA_MAX); // set power level
    radio.setRetries(0,0); // disable auto-retries
    radio.setDataRate(RF24_250KBPS); // set datarate to 250kbps to enhance range
    radio.openWritingPipe(address);

    buffer.deviceId = 1;
    buffer.deviceSubId = 0;
    buffer.deviceType = 1;
    

    #ifndef NODEBUG_PRINT
    radio.printPrettyDetails();
    #endif
    
}

// watchdog interrupt
ISR(WDT_vect){
    wdt_disable();  // disable watchdog
}

void sleepNow(const byte interval) {  
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

#define DEEP_SLEEP_INTERVAL_MULTIPLE    1

void loop() {
    radio.powerUp();
    buffer.seqno = millis();

    for (char i=0;i<3;i++) {
        radio.writeFast(&buffer,sizeof(buffer));
        delay(10);
    }
    radio.powerDown();

    #ifndef NODEBUG_PRINT
    Serial.println(buffer.seqno);
    Serial.println("going to sleep");
    delay(20);
    #endif


    for (char i=0;i<DEEP_SLEEP_INTERVAL_MULTIPLE;i++) sleepNow(0b100001);  // 8 seconds
    //delay(1000); // artificial delay for testing
}