#include "Device.h"
#include <EEPROM.h>
#include "utils.h"

Device::Device(){
    deviceId = getDeviceId();
    button.loop();
    isConfigMode = !button.isOpen();
    radio.begin();

    radio.setAutoAck(false); // disable auto-ack from receivers
    radio.setChannel(NRF_CHANNEL); 
    radio.setCRCLength(RF24_CRC_16); // set CRC length
    radio.setPALevel(RF24_PA_MAX); // set power level
    radio.setRetries(0,0); // disable auto-retries
    radio.setDataRate(RF24_250KBPS); // set datarate to 250kbps to enhance range
    radio.openWritingPipe(RF24BR_BRIDGE_ADDRESS);
    radio.openReadingPipe(0,RF24BR_ACTUATOR_ADDRESS);
    radio.startListening();

    #ifndef NODEBUG_PRINT
    radio.printPrettyDetails();
    #endif

    buffer.srcAdr = deviceId; 
    buffer.srcAdr <<= 16; // deviceId is in two uppoer bytes

    Sensor *sensor = new SensorTempDHT21(0x0001,8);
    sensors.add(sensor);

    if (isConfigMode) {
        #ifndef NODEBUG_PRINT
        Serial.println("Config mode");
        #endif
    } else {
        #ifndef NODEBUG_PRINT
        Serial.println("Normal mode");
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

void Device::normalMode(){
    radio.powerUp();
    
    uint32_t vcc = readVCC();
    buffer.vcc = vcc/33;

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
    ListEntry<Sensor>* i = sensors.getList();
    while (i) {
        i->entry->read(buffer);
        sendBuffer();
        i = i->next;
    }

    #ifndef NODEBUG_PRINT
    Serial.println("[device] ...going to sleep");
    delay(20);
    #endif

    radio.powerDown();
    for (int i=0; i < DEEP_SLEEP_INTERVAL_MULTIPLE; i++) sleep(SleepDuration::DUR_8s);  // sleep 8 seconds
    // delay(1000); // artificial delay for testing
}

void Device::configMode(){
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
    }
}

void Device::loop(){
    button.loop();
    if (isConfigMode) configMode();
    else normalMode();
}

