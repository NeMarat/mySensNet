#ifndef __ENERGO_COUNT__H__
#define __ENERGO_COUNT__H__
//how often send data per cycles
#define PRESCALAR 900 //15 minutes
//delay between saving data to eeprom
#define SAVEINTERVAL 120
//incrementation time, after it summ adds to one of tariffs
#define ADDINTERVAL 10
//ADC pin vith photo resistor
//#define FOTO_PIN A2
#define BATT_PIN A1
//flash adresses for store counted values in rtc flash
#define TARIF_1_ADDR 1
#define TARIF_2_ADDR 5
#define TARIF_3_ADDR 9
#define TARIF_CRC   13
#define SEAL_FLAG   14
#define ADC_LOW_ADDR 15
#define ADC_WDF_ADDR 17
//sensors id's: one value - one sensor
#define T_1_SENS_ID 1
#define T_2_SENS_ID 2
#define T_3_SENS_ID 3
#define TIM_SENS_ID 4
#define WAT_SENS_ID 5
//time intervals in hours for different tariffs
#define T_1_START_1 7
#define T_1_STOP_1 10
#define T_1_START_2 17
#define T_1_STOP_2 21
#define T_2_START_1 23
#define T_2_STOP_1 7
#define T_3_START_1 10
#define T_3_STOP_1 17
#define T_3_START_2 21
#define T_3_STOP_2 23
//how much Vatt*Hour brings one blink
#define VT_H_TIK 0.2
//ADC in volts for battarey: 3/763 for my board
#define BT_V 0.0042
//minimum rtc battarey watch
#define LASTCALLVOLT 2
//3-2-1 * 100 1 volt of battarey is 100 of capacity
#define PERCENTPERVOLT 100

#endif
