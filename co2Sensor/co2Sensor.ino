#include <SPI.h>
#include <MySensor.h>  

#define CHILD_ID_CO 0

byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
unsigned char response[9];

uint64_t SLEEP_TIME = 30000;
uint64_t nowTime;
uint64_t lastSend;
uint32_t coPpm;
uint32_t lastPpm;
uint32_t responseHigh;
uint32_t responseLow;
bool sendTime;
uint8_t i; 
byte crc = 0;

MySensor gw;
boolean metric = true;
MyMessage msgCo(CHILD_ID_CO, V_LEVEL);

void setup() { 
  gw.begin();
  Serial.begin(9600);
  gw.sendSketchInfo("CO2 sensor", "1.1");
  gw.present(CHILD_ID_CO, S_AIR_QUALITY);
  lastSend=millis();
  lastPpm=0;
}

void loop() {
  gw.process();
  nowTime = millis();
  sendTime = nowTime - lastSend > SLEEP_TIME;

  if (sendTime) {
    sendTime = false;
    lastSend=nowTime;
    memset(response, 0, 9);
    Serial.write(cmd, 9);
    Serial.readBytes(response, 9);
    
  for (i = 1; i < 8; i++) { crc+=response[i]; }
  crc = 255 - crc;
  crc++;

  if ( !(response[0] == 0xFF && response[1] == 0x86) ) {
    coPpm=0;
  } else {
    responseHigh = (unsigned int) response[2];
    responseLow = (unsigned int) response[3];
    coPpm=(256*responseHigh) + responseLow;
    if (lastPpm != coPpm) {
      gw.send(msgCo.set(coPpm)); 
      lastPpm=coPpm;
    }
  }
  }
}

