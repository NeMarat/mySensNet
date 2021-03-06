#include <SPI.h>
#include <MySensor.h>  
#include <DHT.h>  
#include <DallasTemperature.h>
#include <OneWire.h>
#include "weatherOregon.h"

#define ONE_WIRE_BUS 6 // Pin where dallase sensor is connected 
#define MAX_ATTACHED_DS18B20 16
#define HUMIDITY_SENSOR_DIGITAL_PIN 4

#define CHILD_ID_ORT  101
#define CHILD_ID_ORH  102
#define CHILD_ID_ORB  103
#define CHILD_ID_HUM  104
#define CHILD_ID_TEMP 105

unsigned long SLEEP_TIME = 80000; //300000;
unsigned long nowTime;
unsigned long lastSend;
float lastTemp;
float lastHumi;
float temp;
float humi;
bool sendTime;

DHT dht;
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 

OregonDecoderV2 orscV2;

MySensor gw;
boolean metric = true;
uint8_t numSensors=0;
word p;
int16_t conversionTime;
float lastTemperature[MAX_ATTACHED_DS18B20];
MyMessage msgDLTmp(0, V_TEMP);
MyMessage oregonT(CHILD_ID_ORT, V_TEMP);
MyMessage oregonH(CHILD_ID_ORH, V_HUM);
MyMessage oregonB(CHILD_ID_ORB, V_VAR2);
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

void oregonrd(void)
{
   static word last;
     pulse = micros() - last;
     last += pulse;
}

void setup() { 
  pinMode(3, INPUT);
  attachInterrupt(1, oregonrd, CHANGE);
  
  sensors.begin();
  //sensors.setWaitForConversion(true);
  numSensors = sensors.getDeviceCount();

  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);
  
  gw.begin();  
  gw.sendSketchInfo("Temp and Oregon reciever", "2.0");
  gw.present(CHILD_ID_ORT, S_TEMP);
  gw.present(CHILD_ID_ORH, S_HUM);
  gw.present(CHILD_ID_ORB, S_CUSTOM);
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  
  // Present all sensors to controller
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {   
     gw.present(i, S_TEMP);
  }
}

void loop() {
  cli();
  p = pulse;
  pulse = 0;
  sei();
  
  //gw.process();
  
  nowTime = millis();
  sendTime = nowTime - lastSend > SLEEP_TIME;

  if (sendTime) {
    sendTime = false;
    lastSend=nowTime;
    
    // dht part
    temp = dht.getTemperature();
    humi = dht.getHumidity();
  
    if (!isnan(temp) && !isnan(humi)) {
      if (humi != lastHumi) {
        lastHumi = humi;
        gw.send(msgHum.set(humi, 1));
      }
  
      if (temp != lastTemp) {
        lastTemp = temp;
        gw.send(msgTemp.set(temp, 1));
      }
    }
    //dallas part
    sensors.begin();
    numSensors = sensors.getDeviceCount();
    sensors.requestTemperatures();
    conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
    delay(conversionTime);
  
    for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {
      gw.present(i, S_TEMP);
      temp = sensors.getTempCByIndex(i);
      if (lastTemperature[i] != temp && temp > -50.0 && temp <= 85.0) {
        msgDLTmp.setSensor(i);
        gw.send(msgDLTmp.set(temp, 1));
        lastTemperature[i]=temp;
      }
    }
  }
  if (p !=0 ) {
    if (orscV2.nextPulse(p)) {
         byte pos;
         const byte * dt = orscV2.getData(pos);
         if(dt[0] == 0xEA && dt[1] == 0x4C) {
           gw.send(oregonT.set(temperature(dt), 1));
           gw.send(oregonH.set(humidity(dt)));
           gw.send(oregonB.set(battery(dt)));
         }
         orscV2.resetDecoder();
    }
  }
}
