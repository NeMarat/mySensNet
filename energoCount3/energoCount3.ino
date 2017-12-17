#define MY_RADIO_NRF24

#include <PZEM004T.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <MySensors.h>
#include "rtcSave.h"
#include "energoCount3.h"

RTC_DS1307 RTC;  
MyMessage wattMsg(WAT_SENS_ID, V_WATT);
MyMessage kwhMsgT1(T_1_SENS_ID, V_KWH);
MyMessage kwhMsgT2(T_2_SENS_ID, V_KWH);
MyMessage kwhMsgT3(T_3_SENS_ID, V_KWH);
MyMessage nodeTime(TIM_SENS_ID, V_VAR5);
MyMessage kwSendTime(TSND_SENS_ID, V_VAR4);
MyMessage voltMsg(VOLT_SENS_ID, V_VOLTAGE);
MyMessage ampMsg(AMP_SENS_ID, V_CURRENT);

PZEM004T pzem(8, 11); // RX, TX
IPAddress ip(172,16,250,5);

float cTime=-1.0;
float watt;
float ct; //current whatt
float tt; //temporary whatt counter
float ta; //temporary whatt counter
float ts; //used for crc
DateTime nowTime;
long reSend=SEND_TIME; //sent curent params every 600 ceconds (adjust by TSND_SENS_ID, V_VAR4)
long creSend=0;  //curent second timer
byte cSec=0;     //current second for saving power data to correct tariff
byte cTarifInterval=0;    //current tarif

void receive(const MyMessage &message) {
  if (message.type==V_VAR5 && message.sensor == TIM_SENS_ID) {
    float gwPulseCount=message.getFloat();
    cTime=gwPulseCount;
  }
  if (message.type==V_VAR4 && message.sensor == TSND_SENS_ID) {
    reSend=message.getFloat();
  }
  if (message.sensor == T_1_SENS_ID) {
    writeFloat(message.getFloat(), TARIF_1_ADDR, &RTC);
  }
  if (message.sensor == T_2_SENS_ID) {
    writeFloat(message.getFloat(), TARIF_2_ADDR, &RTC);
  }
  if (message.sensor == T_3_SENS_ID) {
    writeFloat(message.getFloat(), TARIF_3_ADDR, &RTC);
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

void saveData(float a1, float a2, float a3) {
  writeFloat(a1, TARIF_1_ADDR, &RTC);
  writeFloat(a2, TARIF_2_ADDR, &RTC);
  writeFloat(a3, TARIF_3_ADDR, &RTC);
  ts=a1+a2+a3;
  RTC.writenvram(TARIF_CRC, tarif_crc(&ts));
}

float getSensorVal (byte sensId) {
  switch (sensId) {
    case T_1_SENS_ID :
      return readFloat(TARIF_1_ADDR, &RTC);
    case T_2_SENS_ID :
      return readFloat(TARIF_2_ADDR, &RTC);
    case T_3_SENS_ID :
      return readFloat(TARIF_3_ADDR, &RTC);
    case TIM_SENS_ID: {
      nowTime = RTC.now();
      return float(nowTime.unixtime()); }
    default :
      return -1.0;
  }
}

void newSecond() {
  cSec++;
  creSend++;
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(3), newSecond, CHANGE);
  pzem.setAddress(ip);  
  Wire.begin();
  RTC.begin();
  RTC.writeSqwPinMode(SquareWave1HZ);

  ts = readFloat(TARIF_1_ADDR, &RTC)+readFloat(TARIF_2_ADDR, &RTC)+readFloat(TARIF_3_ADDR, &RTC);
  byte cr = RTC.readnvram(TARIF_CRC);
  byte crg = tarif_crc(&ts);
  
  if (crg != cr) {
    request(T_1_SENS_ID, V_KWH);
    request(T_3_SENS_ID, V_KWH);
    request(T_3_SENS_ID, V_KWH);
    ts = readFloat(TARIF_1_ADDR, &RTC)+readFloat(TARIF_2_ADDR, &RTC)+readFloat(TARIF_3_ADDR, &RTC);
    RTC.writenvram(TARIF_CRC, tarif_crc(&ts));
  }
}

void presentation() {
  sendSketchInfo("Power Meter", "3.0");
  present(T_1_SENS_ID, S_POWER);
  present(T_2_SENS_ID, S_POWER);
  present(T_3_SENS_ID, S_POWER);
  present(TIM_SENS_ID, S_CUSTOM);
  present(TSND_SENS_ID, S_CUSTOM);
  present(WAT_SENS_ID, S_POWER);
  present(VOLT_SENS_ID, S_MULTIMETER);
  present(AMP_SENS_ID, S_MULTIMETER);
}

void loop() {
  if (cTime > 0) {
    RTC.adjust(DateTime(uint32_t(cTime)));
    cTime=-1.0;
    send(nodeTime.set(getSensorVal(TIM_SENS_ID), 0));
  }
  if (cSec > ASK_TIME_INERV) { /*once a minute, if hour and tariff is changed, summ delta to current tariff*/
    byte vTarifInterval=tariff_interval();
    if (cTarifInterval != vTarifInterval) {
      ct = pzem.energy(ip);  //curent counter
      tt = readFloat(TARIF_LAST_AD, &RTC); //last counter
      ta = readFloat(cTarifInterval, &RTC); //tariff counter
      writeFloat(ta+ct-tt, cTarifInterval, &RTC);
      writeFloat(ct, TARIF_LAST_AD, &RTC);
      cTarifInterval=vTarifInterval;
    }
    cSec=0;
  }
  if (creSend > reSend) {
    send(kwhMsgT1.set(getSensorVal(T_1_SENS_ID), 2));
    send(kwhMsgT2.set(getSensorVal(T_2_SENS_ID), 2));
    send(kwhMsgT3.set(getSensorVal(T_3_SENS_ID), 2));
    send(nodeTime.set(getSensorVal(TIM_SENS_ID), 0));
    send(kwSendTime.set(creSend));
    send(voltMsg.set(pzem.voltage(ip), 2));
    send(ampMsg.set(pzem.current(ip), 2));
    send(wattMsg.set(pzem.power(ip), 2));
    creSend=0;
  }

}
