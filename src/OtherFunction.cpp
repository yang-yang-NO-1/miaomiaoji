#include "main.h"
#include "EEPROM.h"

extern void goFront(uint32_t steps, uint16_t wait);
// 按键走纸
void ButtonRun()
{
  if (digitalRead(PIN_KEY) == 0)
  {
    delay(10);
    if (digitalRead(PIN_KEY) == 0)
    {
      // Serial.println("key0");
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
  // 1800对应3.4V
  // PRINTER_BATTERY=(adc-2320)*100/930; //adc读取数值可能不准，请自行调整公式
  PRINTER_BATTERY = (adc - 1800) * 100 / 930; // adc读取数值可能不准，请自行调整公式
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
  /*
  //定时激活IP2306
  else{
    if (millis() - PowerONTime >= 20000){
      digitalWrite(PIN_STAOFF, 0);
      Serial.println("按下电源键");
      PowerONTime = millis();
    }
    else if (millis() - PowerONTime >= 20100){
      digitalWrite(PIN_STAOFF, 1);
      PowerONTime = millis();
      Serial.println("松开电源键");
    }
  }
  */
}

// 获取打印头温度
void HeatTemp()
{
  temp_adc = analogRead(PIN_TEMP);
  Serial.print("打印头温度ADC：");
  Serial.println(temp_adc);
  head_temp = 23;
}

// 缺纸检测

void PaperCheck(){
  if(digitalRead(PIN_PTEST)==1){
    digitalWrite(PIN_VHEN, 0);
    Serial.println("[INFO]no paper");
    PaperSta = 0;
    startBeep();
    delay(500);
    stopBeep();
    delay(100);
    startBeep();
    delay(500);
    stopBeep();
  }
  else{
    PaperSta = 1;
  }
}

