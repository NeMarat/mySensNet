#include <SPI.h>
#include <MySensor.h>  
#include <RCSwitch.h>
#include "weatherOregon.h"

#define CHILD_ID_LED 0
#define CHILD_ID_433 1
#define CHILD_ID_ORG 2
#define LED_PIN 5
#define FADE_DELAY 10

uint8_t led;

MySensor gw;
RCSwitch mySwitch = RCSwitch();
boolean metric = true;
MyMessage lightMsg(CHILD_ID_LED, V_DIMMER);
MyMessage msg433(CHILD_ID_433, V_VAR1);

/***
 *  This method provides a graceful fade up/down effect
 */
void fadeToLevel( int toLevel ) {

  int delta = ( toLevel - led ) < 0 ? -1 : 1;
  
  while ( led != toLevel ) {
    led += delta;
    analogWrite( LED_PIN, (int)(led / 100. * 255) );
    delay( FADE_DELAY );
  }
}

void incomingMessage(const MyMessage &message) {
  if (message.type == V_LIGHT || message.type == V_DIMMER) {
    
    //  Retrieve the power or dim level from the incoming request message
    int requestedLevel = atoi( message.data );
    
    // Adjust incoming level if this is a V_LIGHT variable update [0 == off, 1 == on]
    requestedLevel *= ( message.type == V_LIGHT ? 100 : 1 );
    
    // Clip incoming level to valid range of 0 to 100
    requestedLevel = requestedLevel > 100 ? 100 : requestedLevel;
    requestedLevel = requestedLevel < 0   ? 0   : requestedLevel;
    
    fadeToLevel( requestedLevel );
    
    // Inform the gateway of the current DimmableLED's SwitchPower1 and LoadLevelStatus value...
    gw.send(lightMsg.set(led));
  }
}

void setup() { 
  pinMode(LED_PIN, OUTPUT);
  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2
  attachInterrupt(1, oregonrd, CHANGE);
  gw.begin(incomingMessage);  
  gw.sendSketchInfo("Led driver", "2.0");
  gw.present(CHILD_ID_LED, S_DIMMER);
  gw.present(CHILD_ID_433, S_CUSTOM);
  gw.request(CHILD_ID_LED, V_DIMMER);
}

void loop() {
  gw.process();
  if (mySwitch.available()) {
    gw.send(msg433.set(mySwitch.getReceivedValue()));
    mySwitch.resetAvailable();
  }
  cli();
    word p = pulse;
    pulse = 0;
  sei();
  if (p != 0)
   {
      if (orscV2.nextPulse(p))
      {
         String vData = reportSerial("OSV2", orscV2);
         Serial.println(vData);
      }
   }
}
