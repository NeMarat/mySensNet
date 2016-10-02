#include <SPI.h>
#include <MySensor.h>  
#include <DHT.h>  

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define HUMIDITY_SENSOR_DIGITAL_PIN 5
#define HUMIDITY_SENSOR_POW_PIN 6

// эту константу (typVbg) необходимо откалибровать индивидуально
const float typVbg = 1.1;

unsigned long SLEEP_TIME = 180000;
float lastTemperature;
float lastHumidity;
float temperature;
float humidity;

DHT dht;
MySensor gw;
boolean metric = true;
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

float readVcc() {
  byte i;
  float result = 0.0;
  float tmp = 0.0;

  for (i = 0; i < 2; i++) {
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
    result = result + tmp;
    delay(5);
  }

  result = result / 2;
  return result;
}

void setup() { 
  lastHumidity=0.0;
  lastTemperature=0.0;
  analogReference(DEFAULT);
  pinMode(HUMIDITY_SENSOR_POW_PIN, OUTPUT);
  //pinMode(HUMIDITY_SENSOR_DIGITAL_PIN, INPUT);
  digitalWrite(HUMIDITY_SENSOR_POW_PIN, HIGH);
  //delay(200);
  gw.begin();
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN, DHT::AM2302);

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("Humidity and temp", "2.2");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  
  delay(20);
}

void loop() {
  //gw.begin();
  digitalWrite(HUMIDITY_SENSOR_POW_PIN, HIGH);
  delay(2000);
  gw.process();
  //gw.sendBatteryLevel(readVcc()*10);
  //delay(2*dht.getMinimumSamplingPeriod());
  temperature = dht.getTemperature();
  delay(dht.getMinimumSamplingPeriod());
  humidity = dht.getHumidity();  
  digitalWrite(HUMIDITY_SENSOR_POW_PIN, LOW);
  
  if (!isnan(temperature) && !isnan(humidity)) {
    gw.sendBatteryLevel(readVcc()*10);
    if (humidity != lastHumidity) {
      lastHumidity = humidity;
      gw.send(msgHum.set(humidity, 1));
    }
    if (temperature != lastTemperature) {
      lastTemperature = temperature;
      gw.send(msgTemp.set(temperature, 1));
    }
  }

  gw.sleep(SLEEP_TIME);
}

