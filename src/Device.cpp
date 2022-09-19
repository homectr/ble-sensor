#include "Device.h"
#include <EEPROM.h>
#include "utils.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>

volatile uint8_t interruptIINT0; // device was interrupted from sleep by INT0
volatile uint8_t interruptIINT1; // device was interrupted from sleep by INT1

Device::Device()
{
    // get device id from flashmem, if non is found, then create one
    deviceId = getDeviceId();

    // initialize pin for external interrupts
    pinMode(EXTERNAL_INTERRUPT0_PIN, INPUT_PULLUP);
    pinMode(EXTERNAL_INTERRUPT1_PIN, INPUT_PULLUP);

    // initializ pin managing perpheral power switch
    pinMode(PERIPH_POWER_PIN, OUTPUT);
    connectPeripherals();

    // check button during startup
    button.loop(); // loop must be run in order to read value
    isConfigMode = !button.isOpen();

    // initialize nrf24 radio
    radio.begin();

    radio.setAutoAck(false); // disable auto-ack from receivers
    radio.setChannel(NRF_CHANNEL);
    radio.setCRCLength(RF24_CRC_16); // set CRC length
    radio.setPALevel(RF24_PA_MAX);   // set power level
    radio.setRetries(0, 0);          // disable auto-retries
    radio.setDataRate(RF24_250KBPS); // set datarate to 250kbps to enhance range
    radio.openWritingPipe(RF24BR_BRIDGE_ADDRESS);
    radio.openReadingPipe(0, RF24BR_ACTUATOR_ADDRESS);
    radio.stopListening();

#ifndef NODEBUG_PRINT
// radio.printPrettyDetails();
#endif

    // initialize buffer's source address with device id
    buffer.srcAdr = deviceId;
    buffer.srcAdr <<= 16; // deviceId is in two uppoer bytes

#ifndef NODEBUG_PRINT
    Serial.println("Adding sensors: ");
#endif

    // create DHT11 sensor
    DHT_Unified *dht = new DHT_Unified(DHT11_PIN, DHT11);
    dht->begin();

    Sensor *s;
    s = new SensorDHTTemp(0x0001, dht);
    sensors.add(s);
#ifndef NODEBUG_PRINT
    Serial.print(" 0x");
    Serial.print(s->getId(), HEX);
    Serial.println(" DHT-Temperature");
#endif

    s = new SensorDHTHumidity(0x0002, dht);
    sensors.add(s);
#ifndef NODEBUG_PRINT
    Serial.print(" 0x");
    Serial.print(s->getId(), HEX);
    Serial.println(" DHT-Humidity");
#endif

    s = new SensorContact(0x0003, CONTACT_PIN);
    sensors.add(s);
#ifndef NODEBUG_PRINT
    Serial.print(" 0x");
    Serial.print(s->getId(), HEX);
    Serial.println(" Contact");
#endif

    if (isConfigMode)
    {
        indicator.setMode(IndicatorMode::CONFIG);
        Serial.println(">>> Config mode");
    }
    else
    {
        indicator.setMode(IndicatorMode::NORMAL);
        Serial.println(">>> Normal mode");

        Serial.println("Waiting for peripherals to settle");
        delay(5000);
    }
}

uint16_t Device::getDeviceId()
{
    uint16_t devId;
    uint32_t ecrc;
    EEPROM.get(0, ecrc);
    // read UUID from EEPROM
    EEPROM.get(4, devId);
    // check if UUID is not corrupted
    uint32_t crcmem = crc32((void *)&devId, sizeof(devId));
    // if corrupted (or not existing)
    if (crcmem != ecrc)
    {
#ifndef NODEBUG_PRINT
        Serial.println("UUID CRC ERROR. Creating new UUID");
#endif
        // create UUID
        devId = generate_id16();
        // calculae CRC
        crcmem = crc32((void *)&devId, sizeof(devId));
        // write to EEPROM
        EEPROM.put(0, crcmem);
        EEPROM.put(4, devId);
#ifndef NODEBUG_PRINT
        Serial.print("New UUID=0x");
        Serial.println(devId, HEX);
#endif
    }
    else
    {
#ifndef NODEBUG_PRINT
        Serial.print("Device UUID=0x");
        Serial.println(devId, HEX);
#endif
    }

    return devId;
}

void Device::sendBuffer()
{
#ifndef NODEBUG_PRINT
    Serial.print("memory ");
    Serial.println(freeMemory());
    Serial.print("  > Transmitting data...");
#endif
    radio.stopListening();
    buffer.seqno = millis();

    for (int i = 0; i < 3; i++)
    {
#ifndef NODEBUG_PRINT
        Serial.print(i + 1);
#endif
        radio.writeFast(&buffer, sizeof(buffer));
        delay(10);
    }
    radio.startListening();
#ifndef NODEBUG_PRINT
    Serial.println(" done.");
#endif
}

void Int0ISR(void)
{
    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT0_PIN));
    detachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT1_PIN));
    interruptIINT0 = 1;
}

void Int1ISR(void)
{
    sleep_disable();
    detachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT1_PIN));
    detachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT0_PIN));
    interruptIINT1 = 1;
}

ISR(WDT_vect)
{
    sleep_disable();
    wdt_disable();
}

void Device::sleep(uint16_t sleepCyclesCount)
{
    unsigned char spi_save = SPCR;
    SPCR = 0; // disable SPI
    power_adc_disable();
    power_spi_disable();
    power_timer1_disable();
    power_timer2_disable();
    power_twi_disable();

    // attach to external interrupt
    // check MCU documentation to see which pin is INT0
    EIFR |= 1; // clear old event from interrupt 0
    attachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT0_PIN), Int0ISR, CHANGE);
    interruptIINT0 = 0;

    EIFR |= 2; // clear old event from interrupt 1
    attachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT1_PIN), Int1ISR, CHANGE);
    interruptIINT1 = 0;

    for (uint16_t i = sleepCyclesCount; i > 0; i--)
    {

#ifndef NODEBUG_PRINT
        Serial.print(i);
        Serial.print("  ");
        delay(10);
#endif

        unsigned char sleepDuration = SLEEP_CYCLE_DURATION;
        if (i == 1)
        {
            sleepDuration = SLEEP_DURATION_4s;
            connectPeripherals(); // connect peripherals for the last few seconds, so they can initalize
        }

        MCUSR = 0;                            // reset various flags
        WDTCSR |= 0b00011000;                 // see docs, set WDCE, WDE
        WDTCSR = (1 << WDIE) | sleepDuration; // set WDIE, and appropriate delay
        wdt_reset();
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        power_timer0_disable();

        sleep_mode(); // now goes to Sleep and waits for the interrupt

        power_timer0_enable(); // enable timer0 to enable delay()

        if (interruptIINT0)
        {
            Serial.println("Iterrupt 0");
            delay(10);
            break;
        }
        if (interruptIINT1)
        {
            Serial.println("Iterrupt 1");
            delay(10);
            interruptIINT1 = 0;
            i = 2;            // last 8s of sleep
            _announce = true; // button presssed, device will announce itself
        }
    }
    detachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT0_PIN));
    detachInterrupt(digitalPinToInterrupt(EXTERNAL_INTERRUPT1_PIN));

    power_all_enable();
    SPCR = spi_save; // restore SPI
}

void Device::announceDevices()
{
    ListEntry<Sensor> *i = sensors.getList();
    while (i)
    {
        i->entry->announce(buffer);
        sendBuffer();
        i = i->next;
    }
}

void Device::normalMode()
{
    radio.powerUp();

    uint32_t vcc = readVCC();
    buffer.vcc = vcc / 33;

    buffer.pktType = RFPacketType::DATA;

    if (vcc < 2800)
        indicator.setError(true);
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
    Serial.print("Avail memory ");
    Serial.println(freeMemory());

    // send measurements from all sensors
    Serial.println("Reading sensors");

    ListEntry<Sensor> *i = sensors.getList();
    while (i)
    {
        // do not read sensor if interrupted and it requires initialization time
        if (!(interruptIINT0 && i->entry->requiresInitTime()))
        {
            i->entry->read(buffer);
            sendBuffer();
        }
        else
        {
#ifndef NODEBUG_PRINT
            Serial.print(F("  0x"));
            Serial.print(i->entry->getId(), HEX);
            Serial.println(F(" Requires init time"));
#endif
        }
        i = i->next;
    }

    // if requested, announce sensors
    if (_announce)
    {
#ifndef NODEBUG_PRINT
        Serial.println("[device] Announcing sensors:");
#endif
        announceDevices();
        _announce = false;
    }

    Serial.println("Going to sleep...");
    delay(20);

    // powerdown radio
    radio.powerDown();

    // powering down peripherals
    disconnectPeripherals();

    // deep sleep
    sleep(SLEEP_CYCLES_COUNT);
}

void Device::configMode()
{
    radio.startListening();
#ifndef NODEBUG_PRINT
    if (millis() - aliveTimer > 5000)
    {
        aliveTimer = millis();
        Serial.print("[device-cfg] ms=");
        Serial.print(aliveTimer);
        Serial.println(" is alive");
    }
#endif
    if (radio.available())
    {
        RFActuatorPacket p;
        radio.read(&p, sizeof(p));
#ifndef NODEBUG_PRINT
        Serial.print("data received ");
        Serial.print(p.pktType);
        Serial.print(" ");
        Serial.print(p.dstAdr, HEX);
        Serial.println();
#endif

        if (p.pktType == RFPacketType::SCAN)
        {
            memcpy(buffer.payload, p.payload, sizeof(buffer.payload));
#ifndef NODEBUG_PRINT
            Serial.println("Announcing sensors");
#endif
            announceDevices();
        }
        if (p.pktType == RFPacketType::IDENTIFY)
        {
#ifndef NODEBUG_PRINT
            Serial.println("Identification request reeceived");
#endif

            if ((p.dstAdr && 0xFFFF0000) == (buffer.srcAdr && 0xFFFF0000))
            { // packet is for this device
#ifndef NODEBUG_PRINT
                Serial.println("Identification begins");
#endif

                indicator.setMode(IndicatorMode::IDENTIFICATION, 15000);
            }
        }
    }
}

void Device::disconnectPeripherals()
{
#ifndef NODEBUG_PRINT
    Serial.println("Peripherals: power-down");
    delay(10);
#endif
    digitalWrite(PERIPH_POWER_PIN, 0);
}

void Device::connectPeripherals()
{
#ifndef NODEBUG_PRINT
    Serial.println("Peripherals: power-up");
    delay(10);
#endif

    digitalWrite(PERIPH_POWER_PIN, 1);
}

void Device::loop()
{
    button.loop();
    if (!button.isOpen() && button.stateDuration() > 10000)
    {
        Serial.println("Rebooting");
        for (int i = 0; i < 20; i++)
        {
            indicator.blink();
            delay(100);
        }
        reboot();
    }
    indicator.loop();
    if (isConfigMode)
        configMode();
    else
        normalMode();
}
