#define NOT_AN_INTERRUPT -1

#include <SPI.h> 
#include <DHT.h>  
#include <Arduino.h>
#include <MySensor.h> 

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_PIR 2
#define CHILD_ID_POMP 3
#define HUMIDITY_SENSOR_PIN 3
#define HUMIDITY_POWER_PIN 5
#define PIR_SENSOR_PIN 2
#define POMP_SENSOR_PIN 4
#define NRF_PIN 7
#define NRF_33_PIN 6
#define SLEEP_TIME 1000 // 1 sec to sleep
#define TEMP_MEASURE 300 // measure temp and send every 5 minutes: SLEEP_TIME * TEMP_MEASURE
#define MAX_PIR_RESEND_TIME 180 // do not send PIR data more often than 1 time per 3 minutes

int lastPirSend;
int lastTempMeasured;
byte pompState;
byte lastPompState;
byte nrfPin;
boolean lastPirState;
float lastTemperature;
float lastHumidity;
float temperature;
float humidity;
const float typVbg = 1.1;
boolean tripped;

DHT dht;
MySensor gw;
boolean metric = true;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgPir(CHILD_ID_PIR, V_TRIPPED);
MyMessage msgPomp(CHILD_ID_POMP, V_LIGHT);

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

void gwPresent () {
  if (baTest() > 3.4) {
    nrfPin=NRF_PIN;
  } else {
    nrfPin=NRF_33_PIN;
  };
  digitalWrite(nrfPin, HIGH);
  delay(100);
  gw.begin();
};

void gwSend () {
    if (pompState == HIGH) {
      gw.send(msgPomp.set(1));
    } else {
      gw.send(msgPomp.set(0));
    };
    if (!isnan(temperature) && !isnan(humidity)) {
      if (temperature != lastTemperature) {
        lastTemperature = temperature;
        gw.send(msgTemp.set(temperature, 1));
      }
      if (humidity != lastHumidity) {
        lastHumidity = humidity;
        gw.send(msgHum.set(humidity, 1));
      }
    }
};

void gwSendPir() {
    if (lastPirSend >= MAX_PIR_RESEND_TIME) {
      lastPirSend = 0;
      if (tripped) {
        gw.send(msgPir.set(1));
        tripped=false;
      } else {
        gw.send(msgPir.set(0));
      }
    }
};

void pirWakeUp() {
  if (digitalRead(PIR_SENSOR_PIN) == HIGH) {
    tripped=true; 
  } else {
    tripped=false;
  }
  /*
  gwPresent();
  gwSendPir();
  delay(10);
  digitalWrite(nrfPin, LOW);
  */
};

void setup() { 
  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(POMP_SENSOR_PIN, INPUT);
  pinMode(NRF_PIN, OUTPUT);
  pinMode(HUMIDITY_POWER_PIN, OUTPUT);
  digitalWrite(HUMIDITY_POWER_PIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(PIR_SENSOR_PIN), pirWakeUp, CHANGE);
  gwPresent();
  gw.sendSketchInfo("Outdoor PIR sensor", "2.0");
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_PIR, S_MOTION);
  gw.present(CHILD_ID_POMP, S_LIGHT);
  gw.sendBatteryLevel(int(baTest()*10));
  gw.process();
  dht.setup(HUMIDITY_SENSOR_PIN);
  lastPirSend=0;
  lastTempMeasured=0;
  pompState=LOW;
  lastPompState=LOW;
  digitalWrite(HUMIDITY_POWER_PIN, LOW);
  digitalWrite(nrfPin, LOW);
}

void loop() {
  lastTempMeasured++;
  lastPirSend++;
  if (lastPirState != tripped) {
    lastPirState=tripped;
    gwPresent();
    gwSendPir();
    digitalWrite(nrfPin, LOW);
  }
  pompState=digitalRead(POMP_SENSOR_PIN);
  if (lastPompState != pompState){
    gwPresent();
    if (pompState == HIGH) {
      gw.send(msgPomp.set(1));
    } else {
      gw.send(msgPomp.set(0));
    };
    lastPompState = pompState;
    digitalWrite(nrfPin, LOW);
  };
  if (lastTempMeasured >= TEMP_MEASURE) {
    digitalWrite(HUMIDITY_POWER_PIN, HIGH);
    gwPresent();
    gw.sendBatteryLevel(int(baTest()*10));
    gw.process();
    temperature = dht.getTemperature();
    humidity = dht.getHumidity();
    gwSend(); 
    gwSendPir(); 
    delay(10);
    digitalWrite(HUMIDITY_POWER_PIN, LOW);
    digitalWrite(nrfPin, LOW);
    lastTempMeasured=0;
  }
  gw.sleep(SLEEP_TIME);
}
