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
#include <MySensor.h>
#include "myarveeprom.h"
#include "arduWcount.h"

MySensor gw;
MyMessage volumeMsgCold(CHILD_ID_COLD_W, V_VOLUME);
MyMessage volumeMsgHot(CHILD_ID_HOT_W, V_VOLUME);

MyEeprom mem(MEMSTARTS, MEMENDS, MEMLENGTH, true);

volatile unsigned long coldPinCount=0;
volatile unsigned long hotPinCount=0;
volatile byte cPinState=LOW;
volatile byte hPinState=LOW;
byte t=LOW;
float battV;
volatile int v_prescalar=0;
byte curPower;
const float typVbg = 1.1;

void incomingMessage(const MyMessage &message) {
  if (message.type==V_VOLUME/*V_VAR1*/) {
    unsigned long gwPulseCount=message.getULong();
    if (message.sensor == CHILD_ID_COLD_W) {
      if (gwPulseCount != coldPinCount) {
        coldPinCount = gwPulseCount;
      }
    }
    if (message.sensor == CHILD_ID_HOT_W) {
      if (gwPulseCount != hotPinCount) {
        hotPinCount = gwPulseCount;
      }
    }
  }
}

void checkOut () {
  analogWrite(LED, 32);
  
  digitalWrite(COLDPIN, HIGH); 
  delay(3);
  t=digitalRead(COLDPIN);
  if (t == HIGH && cPinState == LOW) {
    coldPinCount++;
    mem.writeByte(cPinState, COLDPinStateMem);
    mem.writeULong(coldPinCount, COLDCountMem);
  }
  if (cPinState != t) {
    cPinState = t;
  }
  digitalWrite(COLDPIN, LOW);
  digitalWrite(HOTPIN,  HIGH);
  delay(3);
  t=digitalRead(HOTPIN);
  if (t == HIGH && hPinState == LOW) {
    hotPinCount++;
    mem.writeByte(hPinState, HOTPinStateMem);
    mem.writeULong(hotPinCount, HOTCountMem);
  }
  if (hPinState != t) {
    hPinState = t;  
  }
  digitalWrite(HOTPIN,  LOW);
  analogWrite(LED, 0);
}

float baTest () {
  float b = 0.0;
  float tmp = 0.0;
  uint8_t i;
  for (i = 0; i < 4; i++) {
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
    #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
        ADMUX = _BV(MUX5) | _BV(MUX0);
    #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
        ADMUX = _BV(MUX3) | _BV(MUX2);
    #else
        // works on an Arduino 168 or 328
        ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    #endif

    delay(3); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring

    uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
    uint8_t high = ADCH; // unlocks both

    tmp = (high<<8) | low;
    tmp = (typVbg * 1023.0) / tmp;
    b = b + tmp;
    delay(5);
  }

  b = b / i;
  
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
  gw.sendSketchInfo("Water Meter", "2.1");
  //delay(20);
  gw.present(CHILD_ID_COLD_W, S_WATER);
  //delay(20);
  gw.present(CHILD_ID_HOT_W, S_WATER);
  //delay(20);
  gw.sendBatteryLevel(int(battV*10));
  //delay(20);  //to let data be sent
  
  //gw.request(CHILD_ID_COLD_W, V_VOLUME);
  //delay(10);
  //gw.process();
  //delay(20);
  //gw.request(CHILD_ID_HOT_W, V_VOLUME);
  //delay(10);
  //gw.process();
  //delay(20);
  
  gw.send(volumeMsgCold.set(coldPinCount));

  //delay(20);

  gw.send(volumeMsgHot.set(hotPinCount));
 
  gw.process();
  
  //delay(10);
  
  digitalWrite(curPower, LOW);
}

void setup() 
{
  //Serial.begin(115200);
  //cPinState = mem.readByte(COLDPinStateMem);
  //hPinState = mem.readByte(HOTPinStateMem);
  coldPinCount = mem.readULong(COLDCountMem);
  hotPinCount =  mem.readULong(HOTCountMem);
  
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
  analogWrite(LED, 32);
  
  digitalWrite(COLDPIN, HIGH); 
  delay(3);
  cPinState=digitalRead(COLDPIN);
  digitalWrite(COLDPIN, LOW);
  digitalWrite(HOTPIN,  HIGH);
  delay(3);
  hPinState=digitalRead(HOTPIN);
  digitalWrite(HOTPIN, LOW);
  
  

  gw.begin(incomingMessage);
  gw.sendSketchInfo("Water Meter", "2.1");
  gw.present(CHILD_ID_COLD_W, S_WATER);
  gw.present(CHILD_ID_HOT_W, S_WATER);
  //gw.request(CHILD_ID_COLD_W, V_VAR1);
  //gw.request(CHILD_ID_HOT_W, V_VAR1);
  digitalWrite(POW3_3PIN, LOW);
  analogWrite(LED, 0);  
}

void loop() 
{
  //Serial.println(baTest ());
  checkOut ();
  v_prescalar++;
  
  if (v_prescalar >= SLEEPS_SENS) { //its time to radio
    analogWrite(LED, 32);
    //digitalWrite(LED, HIGH);
    senData();
    v_prescalar=0;
    analogWrite(LED, 0);
  }
  //if (pcReceivedCold && pcReceivedHot) {
    gw.sleep(TIME_SLEEP);
  //}
}


