#ifndef MAOPAPERANG_SENSORS_H
#define MAOPAPERANG_SENSORS_H

#include <Arduino.h>

void ButtonRun();
void BatteryPower();
void STAPowerOFF();

bool HeatTemp(bool forceLog = false);
bool IsPaperPresent();
void PaperCheck();
bool PrinterSafetyCheck(bool waitForCooldown);

#endif
