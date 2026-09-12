#ifndef MAOPAPERANG_APP_STATE_H
#define MAOPAPERANG_APP_STATE_H

#include <Arduino.h>

extern float addTime[6];
extern float tmpAddTime;
extern uint8_t heat_density;

extern uint8_t PRINTER_BATTERY;
extern uint16_t adc;
extern uint32_t adcc;

extern uint8_t head_temp;
extern float head_temp_c;
extern uint16_t temp_adc;
extern bool head_temp_valid;
extern bool head_overheat;

extern uint32_t power_down_time;
extern uint32_t PowerOFFTime;
extern uint32_t PowerONTime;

// 1 = paper present, 0 = no paper.
extern int PaperSta;

extern uint8_t *printData;
extern uint32_t printDataCount;
extern int PwrOFFtime;

#endif
