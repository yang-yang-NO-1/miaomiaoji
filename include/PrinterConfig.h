#ifndef MAOPAPERANG_PRINTER_CONFIG_H
#define MAOPAPERANG_PRINTER_CONFIG_H

#include <Arduino.h>

// ======================= Paper sensor =====================
// Mao V2 hardware: LM393 output is pulled up to 3.3 V.
// LOW = paper present, HIGH = no paper.
#define PAPER_PRESENT_LEVEL LOW
#define PAPER_DEBOUNCE_MS 40UL

// ====================== Head temperature ==================
// JX-2R-01 internal NTC: R25=30kΩ, B=3950K.
// Mao V2 divider: 3V3 -> NTC -> TM -> R10(10k) -> GND,
// TM -> R11(1k series) -> GPIO35/TEMP.
#define TEMP_NTC_R25_OHM 30000.0f
#define TEMP_NTC_BETA_K 3950.0f
#define TEMP_PULLDOWN_OHM 10000.0f
#define TEMP_ADC_FULL_SCALE 4095.0f
#define TEMP_ADC_INVALID_LOW 40
#define TEMP_ADC_INVALID_HIGH 4055
#define TEMP_SAMPLE_COUNT 12

// Production safety thresholds.
#define HEAD_TEMP_STOP_C 60.0f
#define HEAD_TEMP_RESUME_C 55.0f
#define TEMP_CHECK_EVERY_ROWS 8U
#define TEMP_LOG_INTERVAL_MS 1000UL

// ======================= Print timing ======================
#define MOTOR_STEP_PER_LINE 3
#define PRINT_TIME 1700
#define PRINT_TIME_ 200
#define MOTOR_TIME 4000

#define kAddTime 0.001
#define STB1_ADDTIME 100
#define STB2_ADDTIME 100
#define STB3_ADDTIME -100
#define STB4_ADDTIME 0
#define STB5_ADDTIME 700
#define STB6_ADDTIME 800

#define Finish_Out 300

// 3 MiB PSRAM print buffer used by ESP32-WROVER.
#define PRINT_DATA_CAPACITY (3UL * 1024UL * 1024UL)

#endif
