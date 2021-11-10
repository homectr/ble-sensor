#include "Device.h"
#include <EEPROM.h>
#include "utils.h"

Device::Device(){
    // get device id from flashmem, if non is found, then create one
    deviceId = getDeviceId();
    
    // initialize pin for external interrupts
    pinMode(EXTERNAL_INTERRUPT_PIN, INPUT_PULLUP);

    // check button during startup
    button.loop(); // loop must be run in order to read value
    isConfigMode = !button.isOpen();
    
    // initialize nrf24 radio
    radio.begin();

    radio.setAutoAck(false); // disable auto-ack from receivers
    radio.setChannel(NRF_CHANNEL); 
    radio.setCRCLength(RF24_CRC_16); // set CRC length
    radio.setPALevel(RF24_PA_MAX); // set power level
    radio.setRetries(0,0); // disable auto-retries
    radio.setDataRate(RF24_250KBPS); // set datarate to 250kbps to enhance range
    radio.openWritingPipe(RF24BR_BRIDGE_ADDRESS);
    radio.openReadingPipe(0,RF24BR_ACTUATOR_ADDRESS);
    radio.stopListening();

    #ifndef NODEBUG_PRINT
    //radio.printPrettyDetails();
    #endif

    // initialize buffer's source address with device id
    buffer.srcAdr = deviceId; 
    buffer.srcAdr <<= 16; // deviceId is in two uppoer bytes

    #ifndef NODEBUG_PRINT
    Serial.print("Adding sensors: ");
    #endif

    // create DHT11 sensor
    DHT_Unified *dht = new DHT_Unified(DHT11_PIN, DHT11);
    dht->begin();

    Sensor *s;
    s = new SensorDHTTemp(0x0001,dht);
    sensors.add(s);
    #ifndef NODEBUG_PRINT
    Serial.print(" DHT-Temperature");
    #endif

    s = new SensorDHTHumidity(0x0002,dht);
    sensors.add(s);
    #ifndef NODEBUG_PRINT
    Serial.print(" DHT-Humidity");
    #endif

    s = new SensorContact(0x0003, CONTACT_PIN);
    sensors.add(s);
    #ifndef NODEBUG_PRINT
    Serial.print(" Contact");
    #endif


    #ifndef NODEBUG_PRINT
    Serial.println();
    #endif


    if (isConfigMode) {
        indicator.setMode(IndicatorMode::CONFIG);
        #ifndef NODEBUG_PRINT
        Serial.println(">>> Config mode");
        #endif
    } else {
        indicator.setMode(IndicatorMode::NORMAL);
        #ifndef NODEBUG_PRINT
        Serial.println(">>> Normal mode");
        #endif
    }

}

uint16_t Device::getDeviceId(){
    uint16_t devId;
    uint32_t ecrc;
    EEPROM.get(0,ecrc);
    // read UUID from EEPROM
    EEPROM.get(4,devId);
    // check if UUID is not corrupted
    uint32_t crcmem = crc32((void*)&devId,sizeof(devId));
    // if corrupted (or not existing)
    if (crcmem != ecrc) {
        #ifndef NODEBUG_PRINT
        Serial.println("UUID CRC ERROR. Creating new UUID");
        #endif
        // create UUID
        devId = generate_id16();
        // calculae CRC
        crcmem = crc32((void*)&devId,sizeof(devId));
        // write to EEPROM
        EEPROM.put(0,crcmem);
        EEPROM.put(4,devId);
        #ifndef NODEBUG_PRINT
        Serial.print("New UUID=0x");
        Serial.println(devId,HEX);
        #endif
    } else {
        #ifndef NODEBUG_PRINT
        Serial.print("Device UUID=0x");
        Serial.println(devId,HEX);
        #endif
    }

    return devId;
}

void Device::sendBuffer(){
    radio.stopListening();
    buffer.seqno = millis();

    for (int i=0;i<3;i++) {
        radio.writeFast(&buffer,sizeof(buffer));
        delay(10);
    }
    radio.startListening();
}

 void Int0ISR(void){
     detachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT_PIN));
 }

void Device::normalMode(){
    radio.powerUp();
    
    uint32_t vcc = readVCC();
    buffer.vcc = vcc/33;

    if (vcc<2800) indicator.setError(true);
    indicator.blink();

    #ifndef NODEBUG_PRINT
    Serial.print("[device] ms=");
    Serial.print(buffer.seqno);
    Serial.print(" button=");
    Serial.print(!button.isOpen());
    Serial.print(" vcc=");
    Serial.print(vcc);
    Serial.print(" vcc%=");
    Serial.println(buffer.vcc);
    #endif

    // send measurements from all sensors
    #ifndef NODEBUG_PRINT
    Serial.println("[device] Reading sensors:");
    #endif

    ListEntry<Sensor>* i = sensors.getList();
    while (i) {
        i->entry->read(buffer);
        sendBuffer();
        i = i->next;
    }

    #ifndef NODEBUG_PRINT
    Serial.println("[device] ...going to sleep");
    #endif

    // attach to external interrupt
    // check MCU documentation to see which pin is INT0
    attachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT_PIN), Int0ISR, CHANGE);
    delay(20);

    // powerdown radio
    radio.powerDown();

    // sleep
    for (int i=0; i < DEEP_SLEEP_INTERVAL_MULTIPLE; i++) sleep(SleepDuration::DUR_8s);  // sleep 8 seconds
    // delay(1000); // artificial delay for testing
}

void Device::configMode(){
    radio.startListening();
    #ifndef NODEBUG_PRINT
    if (millis()-aliveTimer>5000 ){
        aliveTimer = millis();
        Serial.print("[device-cfg] ms=");
        Serial.print(aliveTimer);
        Serial.println(" is alive");
    }
    #endif
    if (radio.available()){
        RFActuatorPacket p;
        radio.read(&p,sizeof(p));
        #ifndef NODEBUG_PRINT
        Serial.print("data received ");
        Serial.print(p.pktType);
        Serial.print(" ");
        Serial.print(p.dstAdr,HEX);
        Serial.println();
        #endif

        if (p.pktType == RFPacketType::SCAN){
            memcpy(buffer.payload, p.payload, sizeof(buffer.payload));
            // announce all sensors
            ListEntry<Sensor>* i = sensors.getList();
            #ifndef NODEBUG_PRINT
            Serial.println("Announcing sensors");
            #endif
            while (i) {
                i->entry->announce(buffer);
                sendBuffer();
                i = i->next;
            }
            
        }
        if (p.pktType == RFPacketType::IDENTIFY){
            #ifndef NODEBUG_PRINT
            Serial.println("Identification request reeceived");
            #endif

            if ((p.dstAdr && 0xFFFF0000) == (buffer.srcAdr && 0xFFFF0000)) { // packet is for this device
                #ifndef NODEBUG_PRINT
                Serial.println("Identification begins");
                #endif

                indicator.setMode(IndicatorMode::IDENTIFICATION,15000);
            }            
        }

    }
}

void Device::loop(){
    button.loop();
    if (!button.isOpen() && button.stateDuration() > 10000) {
        #ifndef NODEBUG_PRINT
        Serial.println("Rebooting");
        #endif
        for(int i=0;i<20;i++){
            indicator.blink();
            delay(100);
        }
        reboot();
    }
    indicator.loop();
    if (isConfigMode) configMode();
    else normalMode();
}

