#include "main.h"
#include "EEPROM.h"
#include <math.h>


static void stopPrintPower()
{
  clearSTB();
  digitalWrite(PIN_VHEN, 0);
  digitalWrite(PIN_MOTOR_AP, 0);
  digitalWrite(PIN_MOTOR_AM, 0);
  digitalWrite(PIN_MOTOR_BP, 0);
  digitalWrite(PIN_MOTOR_BM, 0);
}

static void beepNoPaper()
{
  startBeep();
  delay(180);
  stopBeep();
  delay(100);
  startBeep();
  delay(180);
  stopBeep();
}

static void beepOverheat()
{
  for (uint8_t i = 0; i < 3; ++i)
  {
    startBeep();
    delay(80);
    stopBeep();
    if (i != 2)
    {
      delay(80);
    }
  }
}

// 按键走纸
void ButtonRun()
{
  if (digitalRead(PIN_KEY) == 0)
  {
    delay(10);
    if (digitalRead(PIN_KEY) == 0)
    {
      // 手动走纸保留原行为，便于装纸时将纸送入传感器/打印头。
      goFront(20, MOTOR_TIME);
    }
  }
}

// 电池电量检测
void BatteryPower()
{
  digitalWrite(PIN_BATTEST, 1);
  Serial.print("BatTestPin:");
  Serial.println(digitalRead(PIN_BATTEST));
  uint8_t i;
  for (i = 0; i < 10; i++)
  {
    adc = analogRead(PIN_BATV);
    adcc += adc;
  }
  adc = adcc / 10;
  Serial.print("电池ADC:");
  Serial.println(adc);
  if (adc > 2320)
  {
    adc = 2320;
  }
  PRINTER_BATTERY = (adc - 1800) * 100 / 930;
  adc = 0;
  adcc = 0;

  if (PRINTER_BATTERY > 100)
  {
    PRINTER_BATTERY = 100;
  }
  if (PRINTER_BATTERY < 0)
  {
    PRINTER_BATTERY = 0;
  }
  Serial.print("电量");
  Serial.print(PRINTER_BATTERY);
  Serial.println("%");
  digitalWrite(PIN_BATTEST, 0);
  Serial.print("BatTestPin:");
  Serial.println(digitalRead(PIN_BATTEST));
}

// 自动关机
void STAPowerOFF()
{
  PowerOFFTime = EEPROM.read(PwrOFFtime);
  if (PowerOFFTime - millis() <= 0)
  {
    digitalWrite(PIN_STAOFF, 0);
    delay(100);
    digitalWrite(PIN_STAOFF, 1);
    delay(100);
    digitalWrite(PIN_STAOFF, 0);
    delay(100);
    digitalWrite(PIN_STAOFF, 1);
  }
}

// 获取打印头温度。
// JX-2R-01规格：R25=30kΩ±5%，B=3950K±2%。
// Mao V2实际分压：3V3 -> 打印头内部NTC -> TM -> R10(10k) -> GND；TM经R11(1k)到PIN_TEMP。
bool HeatTemp(bool forceLog)
{
  uint32_t sum = 0;

  // 丢弃一次采样，让ADC采样保持电容先稳定。
  (void)analogRead(PIN_TEMP);
  delayMicroseconds(80);

  for (uint8_t i = 0; i < TEMP_SAMPLE_COUNT; ++i)
  {
    sum += analogRead(PIN_TEMP);
    delayMicroseconds(80);
  }

  temp_adc = (uint16_t)(sum / TEMP_SAMPLE_COUNT);

  if (temp_adc <= TEMP_ADC_INVALID_LOW || temp_adc >= TEMP_ADC_INVALID_HIGH)
  {
    head_temp_valid = false;
    Serial.printf("[TEMP] sensor invalid, ADC=%u\n", temp_adc);
    return false;
  }

  const float adc = (float)temp_adc;
  const float r_ntc = TEMP_PULLDOWN_OHM * (TEMP_ADC_FULL_SCALE - adc) / adc;
  if (!(r_ntc > 0.0f) || !std::isfinite(r_ntc))
  {
    head_temp_valid = false;
    Serial.printf("[TEMP] resistance invalid, ADC=%u\n", temp_adc);
    return false;
  }

  const float t25_k = 25.0f + 273.15f;
  const float inv_t = (1.0f / t25_k) + (logf(r_ntc / TEMP_NTC_R25_OHM) / TEMP_NTC_BETA_K);
  const float temp_c = (1.0f / inv_t) - 273.15f;

  // 超出机芯热敏电阻规格很多时按传感器/线路异常处理。
  if (!std::isfinite(temp_c) || temp_c < -30.0f || temp_c > 100.0f)
  {
    head_temp_valid = false;
    Serial.printf("[TEMP] calculated value invalid, ADC=%u R=%.0f ohm T=%.1fC\n",
                  temp_adc, r_ntc, temp_c);
    return false;
  }

  head_temp_c = temp_c;
  int temp_round = (int)lroundf(temp_c);
  if (temp_round < 0)
  {
    temp_round = 0;
  }
  else if (temp_round > 255)
  {
    temp_round = 255;
  }
  head_temp = (uint8_t)temp_round;
  head_temp_valid = true;

  // 成功采样日志限速，避免过温冷却阶段每250ms刷屏。
  // 故障日志和[SAFETY]事件不受此限制；GET_TEMP可通过forceLog强制输出一次。
  static uint32_t last_temp_log_ms = 0;
  const uint32_t now = millis();
  if (forceLog || last_temp_log_ms == 0 || (uint32_t)(now - last_temp_log_ms) >= TEMP_LOG_INTERVAL_MS)
  {
    last_temp_log_ms = now;
    Serial.printf("[TEMP] ADC=%u R=%.1fk T=%.1fC\n",
                  temp_adc, r_ntc / 1000.0f, head_temp_c);
  }
  return true;
}

bool IsPaperPresent()
{
  return digitalRead(PIN_PTEST) == PAPER_PRESENT_LEVEL;
}

// 缺纸检测：40ms消抖，只在状态变化时提示，避免旧代码在缺纸时不停阻塞鸣叫。
void PaperCheck()
{
  static bool initialized = false;
  static bool last_raw = true;
  static bool stable_state = true;
  static uint32_t changed_at = 0;

  const bool raw = IsPaperPresent();
  const uint32_t now = millis();

  if (!initialized)
  {
    initialized = true;
    last_raw = raw;
    stable_state = raw;
    PaperSta = raw ? 1 : 0;
    changed_at = now;

    if (!raw)
    {
      stopPrintPower();
      Serial.println("[PAPER] no paper");
      beepNoPaper();
    }
    else
    {
      Serial.println("[PAPER] paper ready");
    }
    return;
  }

  if (raw != last_raw)
  {
    last_raw = raw;
    changed_at = now;
  }

  if (raw != stable_state && (uint32_t)(now - changed_at) >= PAPER_DEBOUNCE_MS)
  {
    stable_state = raw;

    if (!stable_state)
    {
      stopPrintPower();
      Serial.println("[PAPER] no paper - heater disabled");
      beepNoPaper();
    }
    else
    {
      Serial.println("[PAPER] paper inserted");
    }
  }

  // 每次都用已消抖状态刷新全局值，避免打印中直接触发安全保护后状态不同步。
  PaperSta = stable_state ? 1 : 0;
}

// 打印前/打印中安全检查。
// 缺纸：立即返回false并关断VH/STB。
// 过温：60°C暂停加热，冷却到55°C后自动恢复；冷却过程中若缺纸则终止。
// 温度传感器断路/短路或计算异常：按故障处理，禁止继续加热。
bool PrinterSafetyCheck(bool waitForCooldown)
{
  if (!IsPaperPresent())
  {
    PaperSta = 0;
    stopPrintPower();
    Serial.println("[SAFETY] print stopped: no paper");
    beepNoPaper();
    return false;
  }
  PaperSta = 1;

  if (!HeatTemp())
  {
    stopPrintPower();
    Serial.println("[SAFETY] print stopped: temperature sensor fault");
    return false;
  }

  if (head_temp_c < HEAD_TEMP_STOP_C)
  {
    if (head_overheat && head_temp_c <= HEAD_TEMP_RESUME_C)
    {
      head_overheat = false;
    }
    return true;
  }

  head_overheat = true;
  stopPrintPower();
  Serial.printf("[SAFETY] head overheat %.1fC, cooling to %.1fC\n",
                head_temp_c, HEAD_TEMP_RESUME_C);
  beepOverheat();

  if (!waitForCooldown)
  {
    return false;
  }

  while (head_overheat)
  {
    delay(250);

    if (!IsPaperPresent())
    {
      PaperSta = 0;
      stopPrintPower();
      Serial.println("[SAFETY] cooling aborted: no paper");
      beepNoPaper();
      return false;
    }

    if (!HeatTemp())
    {
      stopPrintPower();
      Serial.println("[SAFETY] cooling aborted: temperature sensor fault");
      return false;
    }

    if (head_temp_c <= HEAD_TEMP_RESUME_C)
    {
      head_overheat = false;
      Serial.printf("[SAFETY] temperature recovered %.1fC, resume print\n", head_temp_c);
      break;
    }
  }

  return true;
}
