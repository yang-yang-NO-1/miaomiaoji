#include "main.h"
/*
 * 该程序是在小李实验室的基础上进行功能增减的
 */
#include <SPI.h> 
#include "esp_task_wdt.h"
extern void clearSTB();
extern void clearData(void);
extern void paperang_app();
SPIClass printerSPI = SPIClass(HSPI);
SPISettings printerSPISettings = SPISettings(1000000, SPI_MSBFIRST, SPI_MODE0);

void setupPins() {
  pinMode(PIN_MOTOR_AP, OUTPUT);
  pinMode(PIN_MOTOR_AM, OUTPUT);
  pinMode(PIN_MOTOR_BP, OUTPUT);
  pinMode(PIN_MOTOR_BM, OUTPUT);
  digitalWrite(PIN_MOTOR_AP, 0);
  digitalWrite(PIN_MOTOR_AM, 0);
  digitalWrite(PIN_MOTOR_BP, 0);
  digitalWrite(PIN_MOTOR_BM, 0);

  pinMode(PIN_LAT, OUTPUT);
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_SDA, OUTPUT);

  pinMode(PIN_STB1, OUTPUT);
  pinMode(PIN_STB2, OUTPUT);
  pinMode(PIN_STB3, OUTPUT);
  pinMode(PIN_STB4, OUTPUT);
  pinMode(PIN_STB5, OUTPUT);
  pinMode(PIN_STB6, OUTPUT);
  digitalWrite(PIN_STB1, 0);
  digitalWrite(PIN_STB2, 0);
  digitalWrite(PIN_STB3, 0);
  digitalWrite(PIN_STB4, 0);
  digitalWrite(PIN_STB5, 0);
  digitalWrite(PIN_STB6, 0);
  
  pinMode(PIN_KEY, INPUT);     
  pinMode(PIN_BATV, INPUT);    
  pinMode(PIN_PTEST, INPUT);
  pinMode(PIN_TEMP, INPUT);
  pinMode(PIN_BATTEST, OUTPUT);
  digitalWrite(PIN_BATTEST, 0);
  pinMode(PIN_VHEN, OUTPUT);   
  digitalWrite(PIN_VHEN, 0);
  pinMode(PIN_STAOFF, OUTPUT);
  digitalWrite(PIN_STAOFF, 1);
  
  ledcSetup(0, 1000, 8);
  ledcAttachPin(PIN_BUZZER, 0);
  ledcWrite(0, 0);

  printerSPI.begin(PIN_SCK, -1, PIN_SDA, -1);
  printerSPI.setFrequency(2000000);
  clearSTB();
  clearData();

}

void setup(void) {
  Serial.begin(115200);
  setupPins();
  //EEPROM.begin(EEPROM_SIZE);

  printData = (uint8_t*)ps_malloc(3 * 1024 * 1024);   //使用ESP32-WROVER选择此行
  //printData = (uint8_t*)malloc(3 * 1024 * 1024);      //使用ESP32-WROOM / ESP32-SOLO选择此行
  if (!printData) {
    startBeep();
    Serial.println("[ERROR]PSRAM Malloc 失败!\n    请确认esp32模组型号为wrover且须在arduino->开发板中选择ESP32 Wrover Module");
    delay(500);
    stopBeep();
    while (1);
  }
  startBeep();
  delay(50);
  stopBeep();
  //paperang_core0();
  paperang_app();
  //STAPowerOFF();
}


void loop(void) {
  //server.handleClient();
  //  delay(1);
  //PaperCheck();
}
