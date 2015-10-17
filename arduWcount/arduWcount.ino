/*
 * The nrf24l01+ module should be connected to as follows:
 *
 *    rf24    atmega328
 *    ----    ---------
 *    GND  -> GND
 *    VCC  -> 3.3V
 *    CE   -> digital pin 8
 *    CSN  -> digital pin 9
 *    SCK  -> digital pin 13
 *    MOSI -> digital pin 11
 *    MISO -> digital pin 12
 */

#define MARS_OWN_PCB

#include <SPI.h>
#include <EEPROM.h>
#include <MySensor.h>
#include "counterSave.h"
#include "arduWcount.h"

MySensor gw;
MyMessage volumeMsgCold(CHILD_ID_COLD_W, V_VOLUME);
MyMessage volumeMsgHot(CHILD_ID_HOT_W, V_VOLUME);

volatile unsigned long coldPinCount=0;
volatile unsigned long hotPinCount=0;
volatile byte cPinState=LOW;
volatile byte hPinState=LOW;
byte t=LOW;
float battV;
volatile int v_prescalar=0;
byte curPower;

void incomingMessage(const MyMessage &message) {
  if (message.type==V_VOLUME/*V_VAR1*/) {
    unsigned long gwPulseCount=message.getULong();
    if (message.sensor == CHILD_ID_COLD_W) {
      if (gwPulseCount > coldPinCount) {
        coldPinCount = gwPulseCount;
      }
    }
    if (message.sensor == CHILD_ID_HOT_W) {
      if (gwPulseCount > hotPinCount) {
        hotPinCount = gwPulseCount;
      }
    }
  }
}

void checkOut () {
  digitalWrite(LED, HIGH);
  digitalWrite(COLDPIN, HIGH);
  digitalWrite(HOTPIN,  HIGH);
    
  t=digitalRead(COLDPIN);
  if (cPinState != t) {
    cPinState = t;
    coldPinCount++;
    EEPROM.write(COLDPinStateMem, cPinState);
    writeULong(coldPinCount, COLDCountMem);
  }
  t=digitalRead(HOTPIN);
  if (hPinState != t) {
    hPinState = t;
    hotPinCount++;
    EEPROM.write(HOTPinStateMem, hPinState);
    writeULong(hotPinCount, HOTCountMem);  
  }
  
  digitalWrite(COLDPIN, LOW);
  digitalWrite(HOTPIN,  LOW);
  digitalWrite(LED, LOW);
}

float baTest () {
  digitalWrite(BATPINT, HIGH);
  float b = analogRead(BATPIN);
  b += analogRead(BATPIN);
  b += analogRead(BATPIN);
  b += analogRead(BATPIN);
  b = (b/4.0)*BT_V;
  digitalWrite(BATPINT, LOW);
  return (b);
}

inline byte powerPin() { if (baTest () > LOWVOLT) { return(POW3_3PIN); } else { return(POWPASPIN); } }

void senData () {
  battV=baTest();
  
  if (battV > LOWVOLT) { curPower=POW3_3PIN; } else { curPower=POWPASPIN; }
 
  digitalWrite(curPower, HIGH);
  gw.wait(100);
  //delay(100);  //time needs nrf to start
  
  gw.begin(incomingMessage);
  //delay(50);
  gw.sendSketchInfo("Water Meter", "2.0");
  //delay(20);
  gw.present(CHILD_ID_COLD_W, S_WATER);
  //delay(20);
  gw.present(CHILD_ID_HOT_W, S_WATER);
  //delay(20);
  gw.sendBatteryLevel(int((battV-LASTCALLVOLT)*PERCENTPERVOLT));
  //delay(20);  //to let data be sent
  
  gw.request(CHILD_ID_COLD_W, V_VOLUME);
  //delay(10);
  //gw.process();
  //delay(20);
  gw.request(CHILD_ID_HOT_W, V_VOLUME);
  //delay(10);
  //gw.process();
  //delay(20);
  
  gw.send(volumeMsgCold.set(coldPinCount));

  //delay(20);

  gw.send(volumeMsgHot.set(hotPinCount));
  
  //delay(20);
  
  /*
  if (!pcReceivedCold || !pcReceivedHot) {
    digitalWrite(LED, HIGH);
    delay(5);
    digitalWrite(LED, LOW);
    delay(50);
    digitalWrite(LED, HIGH);
    delay(5);
    digitalWrite(LED, LOW);
  }
  */
  //if (!pcReceivedCold) { //Last Pulsecount not yet received from controller, request it again
  //  gw.request(CHILD_ID_COLD_W, S_WATER);
  //  delay(10);
  //}
  //gw.process();
  
  //if (!pcReceivedHot) {
  //  gw.request(CHILD_ID_HOT_W, S_WATER);
  //  delay(10);
  //}
  gw.process();
  
  //delay(10);
  
  digitalWrite(curPower, LOW);
}

void setup() 
{
  //Serial.begin(115200);
  cPinState = EEPROM.read(COLDPinStateMem);
  hPinState = EEPROM.read(HOTPinStateMem);
  coldPinCount = readULong(COLDCountMem);
  hotPinCount =  readULong(HOTCountMem);
  
  //if (coldPinCount == 0) { coldPinCount = 17995; }
  //if (hotPinCount == 0) { hotPinCount = 9799; }
    
  pinMode(COLDPIN, INPUT);
  pinMode(HOTPIN,  INPUT);
  pinMode(BATPINT, OUTPUT); 
  pinMode(BATPIN, INPUT);
  pinMode(POWPASPIN, OUTPUT);
  pinMode(POW3_3PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(BATPINT, LOW);
  digitalWrite(POWPASPIN, LOW);
  digitalWrite(POW3_3PIN, HIGH);
  digitalWrite(LED, HIGH);

  gw.begin(incomingMessage);
  gw.sendSketchInfo("Water Meter", "2.0");
  gw.present(CHILD_ID_COLD_W, S_WATER);
  gw.present(CHILD_ID_HOT_W, S_WATER);
  //gw.request(CHILD_ID_COLD_W, V_VAR1);
  //gw.request(CHILD_ID_HOT_W, V_VAR1);
  digitalWrite(POW3_3PIN, LOW);
  digitalWrite(LED, LOW);  
}

void loop() 
{
  //Serial.println(baTest ());
  checkOut ();
  v_prescalar++;
  
  if (v_prescalar >= SLEEPS_SENS) { //its time to radio
    analogWrite(LED, 64);
    //digitalWrite(LED, HIGH);
    senData();
    v_prescalar=0;
    digitalWrite(LED, LOW);
  }
  //if (pcReceivedCold && pcReceivedHot) {
    gw.sleep(TIME_SLEEP);
  //}
}


