#include <SPI.h>
#include <MySensor.h>  
#include <RCSwitch.h>

#define CHILD_ID_LED 0
#define CHILD_ID_433 1
#define CHILD_ID_FLS 2
#define CHILD_ID_DLY 3
#define LED_PIN 5
#define FADE_DELAY 10

#define MY_NODE_ID 24

uint8_t led;
uint8_t fls=10;
uint16_t dly=50;

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
    gw.wait( FADE_DELAY );
  }
}

void ledFlash () {
  uint8_t leOld = led;
  led = 0;
  analogWrite(LED_PIN, led);
  while (fls > 0) {
    fls--;
    analogWrite(LED_PIN, 255);
    gw.wait(dly);
    analogWrite(LED_PIN, 0);
    gw.wait(dly);
  }
  fadeToLevel(leOld);
}

void incomingMessage(const MyMessage &message) {
  if (message.sensor == CHILD_ID_LED && (message.type == V_LIGHT || message.type == V_DIMMER || message.type == V_PERCENTAGE)) {
    
    //  Retrieve the power or dim level from the incoming request message
    int requestedLevel = atoi( message.data );
    
    // Adjust incoming level if this is a V_LIGHT variable update [0 == off, 1 == on]
    if (message.type == V_LIGHT && requestedLevel == 1) { requestedLevel = 100; } 
    if (message.type == V_LIGHT && requestedLevel == 0) { requestedLevel = 0; }
    
    // Clip incoming level to valid range of 0 to 100
    if (requestedLevel > 100 ) { requestedLevel=100; }
    if (requestedLevel < 0 ) { requestedLevel=0; }
    
    fadeToLevel( requestedLevel );
    // Inform the gateway of the current DimmableLED's SwitchPower1 and LoadLevelStatus value...
    gw.send(lightMsg.set(led));
  }
  if (message.sensor == CHILD_ID_FLS && message.type == V_LIGHT) {
    fls=message.getByte();
  }
  if (message.sensor == CHILD_ID_DLY && message.type == V_VAR2) {
    dly=message.getUInt();
  }
}

void setup() { 
  pinMode(LED_PIN, OUTPUT);
  gw.begin(incomingMessage);  
  gw.sendSketchInfo("Led driver", "2.2");
  gw.present(CHILD_ID_LED, S_DIMMER);
  gw.present(CHILD_ID_433, S_CUSTOM);
  gw.present(CHILD_ID_FLS, S_LIGHT);
  gw.present(CHILD_ID_DLY, S_CUSTOM);
  gw.request(CHILD_ID_LED, V_DIMMER);
  gw.request(CHILD_ID_DLY, V_VAR2);
  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2
}

void loop() {
  gw.process();
  
  if (mySwitch.available()) {
    gw.send(msg433.set(mySwitch.getReceivedValue()));
    mySwitch.resetAvailable();
  }
  if (fls > 0) {
    ledFlash();
  }
    
}
