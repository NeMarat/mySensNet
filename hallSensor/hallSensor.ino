#include <SPI.h>
#include <MySensor.h>  
#include <DHT.h>  
#include <RCSwitch.h>

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_PIR 2
#define CHILD_ID_433 3
#define HUMIDITY_SENSOR_PIN 7
#define PIR_SENSOR_PIN 6

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
RCSwitch mySwitch = RCSwitch();
boolean metric = true;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgPir(CHILD_ID_PIR, V_TRIPPED);
MyMessage msg433(CHILD_ID_433, V_VAR1);

void setup() { 
  pinMode(PIR_SENSOR_PIN, INPUT);
  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2
  gw.begin();
  dht.setup(HUMIDITY_SENSOR_PIN);

  gw.sendSketchInfo("Hall sensor", "2.0");

  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_PIR, S_MOTION);
  gw.present(CHILD_ID_433, S_CUSTOM);
  lastSend=millis();
}

void loop() {
  gw.process();
  nowTime = millis();
  sendTime = nowTime - lastSend > SLEEP_TIME;
  
  if (!tripped) {
    tripped = digitalRead(PIR_SENSOR_PIN) == HIGH;
  } 
  
  if (mySwitch.available()) {
      gw.send(msg433.set(mySwitch.getReceivedValue()));
      mySwitch.resetAvailable();
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
