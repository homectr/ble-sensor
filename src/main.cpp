#include <SPI.h>
#include <RF24.h>
#include <BTLE.h>

RF24 radio(9,10);
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

int i = 0;
float temp=30;
nrf_service_data buf;
char buf2[8] = "HCTRBK";

void loop() {  
  bool a = btle.advertise(0x16, &buf2, sizeof(buf2));
  
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
}