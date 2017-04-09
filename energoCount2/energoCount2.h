#ifndef __ENERGO_COUNT__H__
#define __ENERGO_COUNT__H__
//how often send data per cycles
#define PRESCALAR 900 //15 minutes
//delay between saving data to eeprom
#define SAVEINTERVAL 120
//incrementation time, after it summ adds to one of tariffs
#define ADDINTERVAL 10
//incrementation time, after it current power in watts will be sent
#define WATINTERVAL 30
//flash adresses for store counted values in rtc flash
#define TARIF_1_ADDR 1
#define TARIF_2_ADDR 5
#define TARIF_3_ADDR 9
#define TARIF_CRC   13
#define SEAL_FLAG   14
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

#endif
