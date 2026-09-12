#include "main.h"

uint8_t PRINTER_BATTERY = 90;
uint16_t adc = 0;
uint32_t adcc = 0;

uint8_t head_temp = 23;
float head_temp_c = 23.0f;
uint16_t temp_adc = 0;
bool head_temp_valid = false;
bool head_overheat = false;

uint32_t power_down_time = 3600;
// Preserve the legacy source's uint32_t truncation explicitly, without a compiler warning.
static const uint32_t LEGACY_POWER_OFF_SENTINEL = static_cast<uint32_t>(9999999999ULL);
uint32_t PowerOFFTime = LEGACY_POWER_OFF_SENTINEL;
uint32_t PowerONTime = 0;

int PaperSta = 1;

uint8_t *printData = nullptr;
uint32_t printDataCount = 0;
int PwrOFFtime = 1;

float tmpAddTime = 0.0f;
float addTime[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
uint8_t heat_density = 64;
