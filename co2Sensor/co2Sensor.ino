#include <SPI.h>
#include <Wire.h> 
#include <MySensor.h>  
#include <LiquidCrystal_I2C.h>

#define CHILD_ID_CO 0
#define CHILD_ID_V1 1
#define CHILD_ID_V2 2
#define CHILD_ID_V3 3
#define BATT_PIN    A1

byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
unsigned char response[9];
LiquidCrystal_I2C lcd(0x27, 16, 2);

uint64_t SLEEP_TIME = 20000; //measure every 20 sec
uint16_t REWRITE_TIME = 6; // rewrite display ewery x measuments
uint64_t nowTime;
uint64_t lastSend;
uint32_t coPpm;
uint32_t lastPpm;
uint32_t responseHigh;
uint32_t responseLow;
bool sendTime, reWrite;
uint8_t i; 
byte crc = 0;
String v1, v2, v3, r1, r2;
byte s1, s2;

MySensor gw;
boolean metric = true;
MyMessage msgCo(CHILD_ID_CO, V_LEVEL);

float baTest() {
  uint16_t b = analogRead(BATT_PIN);
  float bv = 5.1*b/1023;
  return(bv);
}

void incomingMessage (const MyMessage &message) {
  char *buf;
  if (message.type==V_VAR1) {  
    v1=message.getString(buf);
  }
  if (message.type==V_VAR2) {  
    v2=message.getString(buf);
  }
  if (message.type==V_VAR3) {  
    v3=message.getString(buf);
  }
  s1 = 16-length(v1)-length(v2)-1;
  s2 = 16-length(v3)-8-1; //CO2=XXXX
  if (s1 < 0){
    s1=1;
    v1=v1.substring(0, min(7, length(v1));
    v2=v2.substring(0, min(8, length(v2));
  }
  if (s2 < 0){
    s2=1;
    v3=v3.substring(0, min(7, length(v3));
  }
}

void setup() { 
  gw.begin(incomingMessage);
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();
  lcd.noBlink();
  lcd.noCursor(); 
  pinMode(BATT_PIN, INPUT);
  gw.sendSketchInfo("CO2 sensor", "1.2");
  gw.present(CHILD_ID_CO, S_AIR_QUALITY);
  gw.present(CHILD_ID_V1, S_CUSTOM);
  gw.present(CHILD_ID_V2, S_CUSTOM);
  gw.present(CHILD_ID_V3, S_CUSTOM);
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
    coPpm=lastPpm;
  } else {
    responseHigh = (unsigned int) response[2];
    responseLow = (unsigned int) response[3];
    coPpm=(256*responseHigh) + responseLow;
    if (abs(lastPpm - coPpm) > 5) {
      gw.send(msgCo.set(coPpm)); 
      gw.sendBatteryLevel(int(baTest()*10));
      lastPpm=coPpm;
    }
  }
  }
  if (reWrite) {
    r1=v1; r2=v3;
    for (byte i=0; i < s1; i++) { r1 = r1+' '; }
    for (byte i=0; i < s2; i++) { r2 = r2+' '; }
  } 
}

