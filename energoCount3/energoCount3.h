#ifndef __ENERGO_COUNT__H__
#define __ENERGO_COUNT__H__

//asc RTC about time every X seconds
#define ASK_TIME_INERV  59

//flash adresses for store counted values in rtc flash
#define TARIF_1_ADDR 1
#define TARIF_2_ADDR 5
#define TARIF_3_ADDR 9
#define TARIF_CRC   13
#define TARIF_LAST_AD 17

//sensors id's: one value - one sensor
#define T_1_SENS_ID 1
#define T_2_SENS_ID 2
#define T_3_SENS_ID 3
#define TIM_SENS_ID 4
#define WAT_SENS_ID 5
#define AMP_SENS_ID 6
#define VOLT_SENS_ID 7
#define TSND_SENS_ID 8

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

#endif
