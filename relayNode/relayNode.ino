#include <MySensor.h>

#define CHILD_ID_RL  1
#define RELAY_ON 1 
#define RELAY_OFF 0 
#define RELAY_PIN 5

MySensor gw;
MyMessage relayMsg(CHILD_ID_RL, V_LIGHT);
bool curState;

void incomingMessage(const MyMessage &message) {
  if (message.type==V_LIGHT) {
     curState=message.getBool()?RELAY_ON:RELAY_OFF;
     digitalWrite(RELAY_PIN, curState);
     gw.send(relayMsg.set(curState));
  } 
}

void setup() { 
  curState=false;
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, 0);
  gw.begin(incomingMessage);  
  gw.sendSketchInfo("Relay", "1.1");
  gw.present(CHILD_ID_RL, S_LIGHT);
  gw.request(CHILD_ID_RL, V_LIGHT);
}

void loop() 
{
  gw.process();
}

