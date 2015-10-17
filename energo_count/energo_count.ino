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
#include <Wire.h>
#include <RTClib.h>
#include <MySensor.h>
#include "rtcSave.h"
#include "energo_count.h"

RTC_DS1307 RTC;   //clock object
MySensor gw;
MyMessage wattMsg(WAT_SENS_ID, V_WATT);
MyMessage kwhMsgT1(T_1_SENS_ID, V_KWH);
MyMessage kwhMsgT2(T_2_SENS_ID, V_KWH);
MyMessage kwhMsgT3(T_3_SENS_ID, V_KWH);
MyMessage nodeTime(TIM_SENS_ID, V_VAR5);

int r_val;        //counted value in this add_interval session
byte c_t_addr;    //current
float t_count;    //tariff value
float watt;
volatile float t1;  //current tarif values
volatile float t2;
volatile float t3;
byte save_delay = 0;
DateTime nowTime;
float ts; //used for crc
boolean scanRf;
int v_prescalar;
byte lastSentSensor;
byte v_add_interval;

void incomingMessage(const MyMessage &message) {
  float gwPulseCount=message.getFloat();
  if (message.type==V_KWH) {
    if (message.sensor == T_1_SENS_ID) {
      t1 = gwPulseCount;
    }
    if (message.sensor == T_2_SENS_ID) {
      t2 = gwPulseCount;
    }
    if (message.sensor == T_3_SENS_ID) {
      t3 = gwPulseCount;
    }
  }
  if (message.type==V_VAR5) {
    if (message.sensor == TIM_SENS_ID) {
      RTC.adjust(DateTime(uint32_t(gwPulseCount)));
    }
  }
}

inline byte tarif_crc (float *crc_data) {
  return (*((byte*)crc_data + 0) ^ *((byte*)crc_data + 1) ^ *((byte*)crc_data + 2) ^ *((byte*)crc_data + 3));
}

byte tariff_interval () {
  nowTime = RTC.now();
  byte h = nowTime.hour();
  if ((h >= T_1_START_1 && h < T_1_STOP_1) 
   || (h >= T_1_START_2 && h < T_1_STOP_2)) {
     return TARIF_1_ADDR;
  }
  if (h >= T_2_START_1 || h < T_2_STOP_1) {
    return TARIF_2_ADDR;
  }
  if ((h >= T_3_START_1 && h < T_3_STOP_1) 
   || (h >= T_3_START_2 && h < T_3_STOP_2)) {
     return TARIF_3_ADDR;
  }
}

inline float check_bat () {
  float b = analogRead(BATT_PIN);
  b += analogRead(BATT_PIN);
  b += analogRead(BATT_PIN);
  b += analogRead(BATT_PIN);
  b = b/4*BT_V;
  return(b);
  //digitalWrite(BATT_PIN, LOW);
}

void saveData(float a1, float a2, float a3) {
  writeFloat(a1, TARIF_1_ADDR, &RTC);
  writeFloat(a2, TARIF_2_ADDR, &RTC);
  writeFloat(a3, TARIF_3_ADDR, &RTC);
  ts=t1+t2+t3;
  RTC.writenvram(TARIF_CRC, tarif_crc(&ts));
}

float getSensorVal (byte sensId) {
  switch (sensId) {
    case T_1_SENS_ID :
      return t1;
    case T_2_SENS_ID :
      return t2;
    case T_3_SENS_ID :
      return t3;
    case TIM_SENS_ID: {
      nowTime = RTC.now();
      return float(nowTime.unixtime()); }
    default :
      return -1.0;
  }
}

ISR (TIMER1_COMPA_vect) { //every second by interrupt code
  v_prescalar++;
  save_delay++;
  v_add_interval++;
}

void check_led() {
  r_val++;
}
 
void setup()
{  
  pinMode(BATT_PIN, INPUT);
  
  //Serial.begin(9600);   
  Wire.begin();
  RTC.begin();
  //if (! RTC.isrunning()) { ;
  //  Serial.println("RTC is NOT running!");
    /* following line sets the RTC to the date & time this sketch was compiled */
   //RTC.adjust(DateTime(__DATE__, __TIME__));
  //}
  
  gw.begin(incomingMessage);
  gw.sendSketchInfo("Power Meter", "2.0");
  gw.present(T_1_SENS_ID, S_POWER);
  //delay(20);
  gw.present(T_2_SENS_ID, S_POWER);
  //delay(20);
  gw.present(T_3_SENS_ID, S_POWER);
  //delay(20);
  gw.present(TIM_SENS_ID, S_CUSTOM);
  //delay(20);
  gw.present(WAT_SENS_ID, S_POWER);
  //delay(20);
  
  t1 = readFloat(TARIF_1_ADDR, &RTC);
  t2 = readFloat(TARIF_2_ADDR, &RTC);
  t3 = readFloat(TARIF_3_ADDR, &RTC);
  /*
  t1 = 1628080.0;
  t2 = 1265050.0;
  t3 = 1975560.0;
  RTC.adjust(DateTime(uint32_t(1443290580)));
  */
  ts = t1+t2+t3;
  byte cr = RTC.readnvram(TARIF_CRC);
  byte crg = tarif_crc(&ts);
  
  if (crg != cr) {
    gw.request(T_1_SENS_ID, V_KWH);
    //delay(30);
    gw.request(T_3_SENS_ID, V_KWH);
    //delay(30);
    gw.request(T_3_SENS_ID, V_KWH);
    //delay(30);
    saveData(t1, t2, t3);
  }
  
  v_prescalar=0;
  save_delay=0;
  v_add_interval=0;
  r_val=0;  
  watt=0;
  
  noInterrupts();
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536) : 1 time per second
  // turn on CTC mode
  TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
  
  attachInterrupt(0, check_led, FALLING); //falling voltage on pin2
  
}

void loop()
{  
  gw.process();
  if (v_prescalar >= PRESCALAR) {
    gw.begin(incomingMessage);
    //delay(20);
    gw.sendBatteryLevel(int((check_bat ()-LASTCALLVOLT)*PERCENTPERVOLT));
    //delay(20);  //to let data be sent
    gw.send(kwhMsgT1.set(t1, 2));
    //delay(20);
    gw.send(kwhMsgT2.set(t2, 2));
    //delay(20);
    gw.send(kwhMsgT3.set(t3, 2));
    //delay(20);
    gw.send(wattMsg.set(watt, 2));
    //delay(20);
    nowTime = RTC.now();
    gw.send(nodeTime.set(nowTime.unixtime()));
    //delay(20);
    v_prescalar=0;
  }
  if (v_add_interval >= ADDINTERVAL) {
    c_t_addr = tariff_interval();
    if (c_t_addr == TARIF_1_ADDR) {
      t1 += VT_H_TIK * r_val;
    } 
    if (c_t_addr == TARIF_2_ADDR) {
      t2 += VT_H_TIK * r_val;
    }  
    if (c_t_addr == TARIF_3_ADDR) {
      t3 += VT_H_TIK * r_val;
    }
    if (r_val > 0) {
      watt = (VT_H_TIK * r_val) / (ADDINTERVAL / 3600.0);
    } else {
      watt = 0;
    }
    gw.send(wattMsg.set(watt, 2));
    v_add_interval=0;
    r_val=0;
  }
  if (save_delay >= SAVEINTERVAL) {
    saveData(t1, t2, t3);
    save_delay = 0;
  }
}

