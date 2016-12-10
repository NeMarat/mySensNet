#include <SPI.h>
#include <MySensor.h>  
#include <DHT.h>  
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define CHILD_ID_HUM 0  //влажность
#define CHILD_ID_TEMP 1 //температура
#define CHILD_ID_PIR 2  //датчик движения
#define CHILD_ID_SND 3  //количество звонков
#define CHILD_ID_DLY 4  //перерыв между ними
#define CHILD_ID_TON 5  //тональность звонка
#define CHILD_ID_BMP 6  //давление
#define HUMIDITY_SENSOR_PIN 7 //dht22
#define PIR_SENSOR_PIN 6      //датчик движения
#define SND_PIEZO_PIN 5       //пищалка

unsigned long SLEEP_TIME = 180000;
unsigned long nowTime;
unsigned long lastSend;
bool sendTime;
float lastTemperature;
float lastHumidity;
float temperature;
float humidity;
boolean tripped;

DHT dht;
MySensor gw;
boolean metric = true;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgPir(CHILD_ID_PIR, V_TRIPPED);
MyMessage msgPres(CHILD_ID_BMP, V_PRESSURE);

void setup() { 
  pinMode(PIR_SENSOR_PIN, INPUT);
  gw.begin();
  dht.setup(HUMIDITY_SENSOR_PIN);

  gw.sendSketchInfo("Hall sensor", "2.0");

  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_PIR, S_MOTION);
  gw.present(CHILD_ID_IR, S_CUSTOM);
  lastSend=millis();
  irReady=false;
}

void loop() {
  gw.process();
  nowTime = millis();
  sendTime = nowTime - lastSend > SLEEP_TIME;
  
  if (!tripped) {
    tripped = digitalRead(PIR_SENSOR_PIN) == HIGH;
  } 

  if (sendTime) {
    sendTime = false;
    lastSend=nowTime;
  
    temperature = dht.getTemperature();
    humidity = dht.getHumidity();
  
    if (!isnan(temperature) && !isnan(humidity)) {
      if (humidity != lastHumidity) {
        lastHumidity = humidity;
        gw.send(msgHum.set(humidity, 1));
      }
  
      if (temperature != lastTemperature) {
        lastTemperature = temperature;
        gw.send(msgTemp.set(temperature, 1));
      }
    }
  
    if (tripped) {
      gw.send(msgPir.set(1));
      tripped=false;
    } else {
      gw.send(msgPir.set(0));
    }
  }
}
