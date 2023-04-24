#ifndef _MAIN_H_
#define _MAIN_H_
#include <Arduino.h>
// 打印头电机控制引脚
/*
#define PIN_MOTOR_AP 19
#define PIN_MOTOR_AM 21
#define PIN_MOTOR_BP 22
#define PIN_MOTOR_BM 23
*/
// 改版之后的电机控制引脚
// 打印头电机控制引脚
#define PIN_MOTOR_AP 23
#define PIN_MOTOR_AM 22
#define PIN_MOTOR_BP 21
#define PIN_MOTOR_BM 19

// 打印头数据引脚
#define PIN_LAT 18
#define PIN_SCK 5
#define PIN_SDA 4
#define PIN_STB1 26
#define PIN_STB2 25
#define PIN_STB3 33
#define PIN_STB4 32
#define PIN_STB5 14
#define PIN_STB6 27
// 蜂鸣器引脚
#define PIN_BUZZER 12
#define BUZZER_FREQ 2000
#define startBeep() ledcWrite(0, 127)
#define stopBeep() ledcWrite(0, 0)
// 按键走纸
#define PIN_KEY 36
// 打印机状态指示灯引脚
// #define PIN_STATUS 13
// 打印头电源升压控制引脚
#define PIN_VHEN 2
// 电池检测控制引脚
#define PIN_BATTEST 13
#define PIN_BATV 39
// 打印头温度检测引脚
#define PIN_TEMP 35
// 缺纸侦测引脚
#define PIN_PTEST 34
// 自动关机引脚
#define PIN_STAOFF 15
// 打印头电机参数
#define MOTOR_STEP_PER_LINE 3
#define PRINT_TIME 1700
#define PRINT_TIME_ 200
#define MOTOR_TIME 4000
extern float addTime[6];
extern float tmpAddTime;
// 根据打印头打印效果修改
#define kAddTime 0.001   // 点数-增加时间系数, 见sendData函数
#define STB1_ADDTIME 100 // STB1额外打印时间,下面类似, 单位: 微秒
#define STB2_ADDTIME 100 // 根据打印头实际情况修改
#define STB3_ADDTIME -100
#define STB4_ADDTIME 0
#define STB5_ADDTIME 700
#define STB6_ADDTIME 800
extern uint8_t heat_density;

extern uint8_t PRINTER_BATTERY; // 打印机电量
extern uint16_t adc;
extern uint32_t adcc;

extern uint8_t head_temp; // 打印头温度
extern uint16_t temp_adc;

extern uint32_t power_down_time; // 关机时间
extern uint32_t PowerOFFTime;
extern uint32_t PowerONTime;

extern int PaperSta; // zhuangtai

#define Finish_Out 300 // 打印完毕出纸长度，单位：点，8点/mm。

// 打印头驱动部分
extern uint8_t *printData; //打印数据缓存
extern uint32_t printDataCount;

// EEPROM地址设置
// #define EEPROM_SIZE 64    //EEPROM大小
extern int PwrOFFtime;

#endif