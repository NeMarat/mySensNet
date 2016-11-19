#include <MySensor.h>

#define CHILD_ID_RL 1
#define CHILD_ID_TP 2
#define RELAY_ON 1 
#define RELAY_OFF 0 
#define RELAY_PIN 5

MySensor gw;
MyMessage relayMsg(CHILD_ID_RL, V_LIGHT);
MyMessage dimrMsg(CHILD_ID_RL, V_DIMMER);
bool curState;
bool relayType;
uint8_t freq;  //seconds in minute which heater will be turned on. useful only for oil (and same) heaters
uint16_t lastMs;
uint16_t v_prescalar;

void incomingMessage(const MyMessage &message) {
  if (message.type==V_LIGHT) {
     curState=message.getBool()?RELAY_ON:RELAY_OFF;
     digitalWrite(RELAY_PIN, curState);
     gw.send(relayMsg.set(curState));
  } 
  if (message.type==V_VAR1) {
    relayType=message.getBool()?RELAY_ON:RELAY_OFF;
    if (relayType == 1) {
      gw.present(CHILD_ID_RL, S_DIMMER);
      gw.request(CHILD_ID_RL, V_DIMMER);
    } else {
      gw.present(CHILD_ID_RL, S_LIGHT);
      gw.request(CHILD_ID_RL, V_LIGHT);
    }
  }
  if (message.type==V_DIMMER) {
     freq=message.getInt();
     freq=min(freq, 59);
     freq=max(freq, 0);
     gw.send(dimrMsg.set(freq));
  }
}

ISR (TIMER1_COMPA_vect) { //every second by interrupt code
  v_prescalar++;
}

void setup() { 
  curState=false;
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, 0);
  gw.begin(incomingMessage);  
  gw.sendSketchInfo("Relay", "2.1");
  gw.present(CHILD_ID_RL, S_LIGHT);
  gw.request(CHILD_ID_RL, V_LIGHT);
  gw.present(CHILD_ID_TP, S_CUSTOM);
  gw.request(CHILD_ID_TP, V_VAR1);
  
  v_prescalar=0;
  
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
}

void loop() 
{
  gw.process();
  if (relayType == 1) {
    if (freq >= v_prescalar && freq > 0) {
      if (curState == 0) {
        curState=1;
        digitalWrite(RELAY_PIN, curState);
      }
    } else {
      if (curState == 1) {
        curState=0;
        digitalWrite(RELAY_PIN, curState);
      }
    }
  }
  if (v_prescalar > 59) {
    v_prescalar=0;
  }
}

