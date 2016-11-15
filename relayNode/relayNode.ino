#include <MySensor.h>

#define CHILD_ID_RL 1
#define CHILD_ID_TP 2
#define RELAY_ON 1 
#define RELAY_OFF 0 
#define RELAY_PIN 5

MySensor gw;
MyMessage relayMsg(CHILD_ID_RL, V_LIGHT);
bool curState;
bool relayType;
uint8_t freq;  //seconds in minute which heater will be turned on. useful only for oil (and same) heaters

void incomingMessage(const MyMessage &message) {
  if (message.type==V_LIGHT) {
     curState=message.getBool()?RELAY_ON:RELAY_OFF;
     digitalWrite(RELAY_PIN, curState);
     gw.send(relayMsg.set(curState));
  } 
  if (message.type==V_CUSTOM) {
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
     digitalWrite(RELAY_PIN, curState);
     gw.send(relayMsg.set(curState));
  }
}

void setup() { 
  curState=false;
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, 0);
  gw.begin(incomingMessage);  
  gw.sendSketchInfo("Relay", "2.1");
  gw.present(CHILD_ID_RL, S_LIGHT);
  gw.request(CHILD_ID_RL, V_LIGHT);
  gw.present(CHILD_ID_TP, S_CUSTO(==V_CUSTOM)M);
  gw.request(CHILD_ID_TP, V_CUSTOM);
  
}

void loop() 
{
  gw.process();
}

