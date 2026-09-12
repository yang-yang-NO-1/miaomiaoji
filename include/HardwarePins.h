#ifndef MAOPAPERANG_HARDWARE_PINS_H
#define MAOPAPERANG_HARDWARE_PINS_H

#include <Arduino.h>

// ========================= Motor =========================
#define PIN_MOTOR_AP 23
#define PIN_MOTOR_AM 22
#define PIN_MOTOR_BP 21
#define PIN_MOTOR_BM 19

// ====================== Thermal head =====================
#define PIN_LAT 18
#define PIN_SCK 5
#define PIN_SDA 4
#define PIN_STB1 26
#define PIN_STB2 25
#define PIN_STB3 33
#define PIN_STB4 32
#define PIN_STB5 14
#define PIN_STB6 27
#define PIN_VHEN 2

// ========================= Sensors ========================
#define PIN_KEY 36
#define PIN_BATTEST 13
#define PIN_BATV 39
#define PIN_TEMP 35
#define PIN_PTEST 34
#define PIN_STAOFF 15

// ========================== Buzzer ========================
#define PIN_BUZZER 12
#define BUZZER_FREQ 2000

inline void startBeep() { ledcWrite(0, 127); }
inline void stopBeep() { ledcWrite(0, 0); }

#endif
