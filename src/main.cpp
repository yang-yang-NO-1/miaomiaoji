#include "main.h"
#include <SPI.h>

SPIClass printerSPI = SPIClass(HSPI);
SPISettings printerSPISettings = SPISettings(1000000, SPI_MSBFIRST, SPI_MODE0);

static void setupPins()
{
  pinMode(PIN_MOTOR_AP, OUTPUT);
  pinMode(PIN_MOTOR_AM, OUTPUT);
  pinMode(PIN_MOTOR_BP, OUTPUT);
  pinMode(PIN_MOTOR_BM, OUTPUT);
  digitalWrite(PIN_MOTOR_AP, LOW);
  digitalWrite(PIN_MOTOR_AM, LOW);
  digitalWrite(PIN_MOTOR_BP, LOW);
  digitalWrite(PIN_MOTOR_BM, LOW);

  pinMode(PIN_LAT, OUTPUT);
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_SDA, OUTPUT);

  pinMode(PIN_STB1, OUTPUT);
  pinMode(PIN_STB2, OUTPUT);
  pinMode(PIN_STB3, OUTPUT);
  pinMode(PIN_STB4, OUTPUT);
  pinMode(PIN_STB5, OUTPUT);
  pinMode(PIN_STB6, OUTPUT);
  clearSTB();

  pinMode(PIN_KEY, INPUT);
  pinMode(PIN_BATV, INPUT);
  pinMode(PIN_PTEST, INPUT);
  pinMode(PIN_TEMP, INPUT);

  pinMode(PIN_BATTEST, OUTPUT);
  digitalWrite(PIN_BATTEST, LOW);
  pinMode(PIN_VHEN, OUTPUT);
  digitalWrite(PIN_VHEN, LOW);
  pinMode(PIN_STAOFF, OUTPUT);
  digitalWrite(PIN_STAOFF, HIGH);

  ledcSetup(0, 1000, 8);
  ledcAttachPin(PIN_BUZZER, 0);
  stopBeep();

  printerSPI.begin(PIN_SCK, -1, PIN_SDA, -1);
  printerSPI.setFrequency(2000000);
  clearData();
}

void setup()
{
  Serial.begin(115200);
  setupPins();

  printData = static_cast<uint8_t *>(ps_malloc(PRINT_DATA_CAPACITY));
  if (!printData)
  {
    startBeep();
    Serial.println("[ERROR] PSRAM Malloc失败：请确认ESP32模组为WROVER，并选择ESP32 Wrover开发板配置");
    delay(500);
    stopBeep();
    while (true)
    {
      delay(1000);
    }
  }

  startBeep();
  delay(50);
  stopBeep();

  paperang_app();
}

void loop()
{
  // paperang_app() owns the main service loop and does not return.
}
