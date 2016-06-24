#include <SPI.h>
#include <MySensor.h>  
#include <DHT.h>  
#include <IRLib.h>

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_PIR 2
#define CHILD_ID_IR 3
#define HUMIDITY_SENSOR_PIN 7
#define PIR_SENSOR_PIN 6
#define kHz 38;

unsigned long SLEEP_TIME = 180000;
unsigned long nowTime;
unsigned long lastSend;
bool sendTime;
float lastTemperature;
float lastHumidity;
float temperature;
float humidity;
boolean tripped;
boolean irReady;
uint16_t irMsg1, irMsg2;

DHT dht;
MySensor gw;
boolean metric = true;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgPir(CHILD_ID_PIR, V_TRIPPED);
MyMessage msgIr(CHILD_ID_IR, V_VAR1);

void irRecive(const MyMessage &message) {
IRsend My_Sender;
int m = message.getInt();
if (message.sensor == CHILD_ID_IR) {
  if (m == 1) {
    My_Sender.send(NEC,0x9090, 32);
  }
  if (m == 0) {
    My_Sender.send(NEC,0x8090030A, 32);
  }
}

}

void setup() { 
  pinMode(PIR_SENSOR_PIN, INPUT);
  gw.begin(irRecive);
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
