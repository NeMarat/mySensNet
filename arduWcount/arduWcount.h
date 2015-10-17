#ifndef _ARDU_WCOUNT_H_
#define _ARDU_WCOUNT_H_

#define CHILD_ID_COLD_W 0
#define CHILD_ID_HOT_W 1
#define CHILD_ID_HUMID 2

//connection pins
#define COLDPIN 3  
#define HOTPIN  2
#define BATPIN    A0  //pin to test ADC
#define BATPINT   A1  //pin to turn on and test
#define POWPASPIN A3
#define POW3_3PIN A2
#define LED        6

#define BT_V      0.0035
#define LOWVOLT   3.9
#define PERCENTPERVOLT 62.5 // 100 / (max volt - min)
#define LASTCALLVOLT 2.9 

#define TIME_SLEEP 4000 //4 seconds
#define SLEEPS_SENS 300 //20 min if wake ups ewery 4 seconds

//EEPROM addresses for data save
// addr 0 is for future possible use
#define COLDPinStateMem 512 //1
#define HOTPinStateMem  513 //2
#define COLDCountMem    514 //3 //datalength - 4 bytes: long type is used
#define HOTCountMem     518 //7 //datalength - 4 bytes: long type is used

#endif

