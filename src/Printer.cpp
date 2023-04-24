#include "main.h"
#include <SPI.h>

extern SPIClass printerSPI;
extern SPISettings printerSPISettings;
extern void clearSTB();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 步进电机驱动部分

uint8_t motorTable[8][4] = {
    {1, 0, 0, 0},
    {1, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}};
uint8_t motorPos = 0;

void goFront(uint32_t steps, uint16_t wait)
{
  ++steps;
  while (--steps)
  {
    digitalWrite(PIN_MOTOR_AP, motorTable[motorPos][0]);
    digitalWrite(PIN_MOTOR_AM, motorTable[motorPos][1]);
    digitalWrite(PIN_MOTOR_BP, motorTable[motorPos][2]);
    digitalWrite(PIN_MOTOR_BM, motorTable[motorPos][3]);
    ++motorPos;
    if (motorPos == 8)
    {
      motorPos = 0;
    }
    delayMicroseconds(wait);
  }
  digitalWrite(PIN_MOTOR_AP, 0);
  digitalWrite(PIN_MOTOR_AM, 0);
  digitalWrite(PIN_MOTOR_BP, 0);
  digitalWrite(PIN_MOTOR_BM, 0);
}

void goFront1()
{
  digitalWrite(PIN_MOTOR_AP, motorTable[motorPos][0]);
  digitalWrite(PIN_MOTOR_AM, motorTable[motorPos][1]);
  digitalWrite(PIN_MOTOR_BP, motorTable[motorPos][2]);
  digitalWrite(PIN_MOTOR_BM, motorTable[motorPos][3]);
  ++motorPos;
  if (motorPos == 8)
  {
    motorPos = 0;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void clearAddTime()
{
  addTime[0] = addTime[1] = addTime[2] = addTime[3] = addTime[4] = addTime[5] = 0;
}

// SPI发送数据函数
void sendData(uint8_t *dataPointer)
{
  for (uint8_t i = 0; i < 6; ++i)
  {
    for (uint8_t j = 0; j < 8; ++j)
    {
      addTime[i] += dataPointer[i * 8 + j];
    }
    tmpAddTime = addTime[i] * addTime[i];
    addTime[i] = kAddTime * tmpAddTime;
  }
  printerSPI.beginTransaction(printerSPISettings);
  printerSPI.transfer(dataPointer, 48);
  printerSPI.endTransaction();

  digitalWrite(PIN_LAT, 0);
  delayMicroseconds(1);
  digitalWrite(PIN_LAT, 1);
}

void clearData(void)
{
  printerSPI.beginTransaction(printerSPISettings);
  for (uint8_t i = 0; i < 48; ++i)
  {
    printerSPI.transfer(0);
  }
  printerSPI.endTransaction();
  clearAddTime();
  digitalWrite(PIN_LAT, 0);
  delayMicroseconds(1);
  digitalWrite(PIN_LAT, 1);
}

/* 默认打印头步进电机转4步，打印机走纸一像素的距离，如果发现打印的文字长度过长或过扁，
请修改startPrint函数中的goFront1()函数出现位置和次数，这个函数的作用是使打印头步进电机走1步 */
void startPrint()
{
  if (PaperSta == 1)
  {
    static unsigned char motor_add = 0;
    startBeep();
    delay(200);
    stopBeep();

    Serial.println("[INFO]正在打印...");
    Serial.printf("[INFO]共%u行\n", printDataCount / 48);
    digitalWrite(PIN_VHEN, 1);
    // digitalWrite(PIN_STATUS, 1);

    for (uint32_t pointer = 0; pointer < printDataCount; pointer += 48)
    {

      motor_add++;
      if (motor_add != 40)
      {
        delayMicroseconds((PRINT_TIME) * ((double)heat_density / 100));
        // goFront1();
      }
      else
      {
        motor_add = 0;
      }
      clearAddTime();
      sendData(printData + pointer);
      for (char l = 0; l < 4; l++)//重复打印4次，否则直线会向下偏移
      {
        digitalWrite(PIN_STB1, 1);
        delayMicroseconds((PRINT_TIME + addTime[0] + STB1_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB1, 0);
        delayMicroseconds(PRINT_TIME_);

        digitalWrite(PIN_STB2, 1);
        delayMicroseconds((PRINT_TIME + addTime[1] + STB2_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB2, 0);
        delayMicroseconds(PRINT_TIME_);
        // goFront1();

        digitalWrite(PIN_STB3, 1);
        delayMicroseconds((PRINT_TIME + addTime[2] + STB3_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB3, 0);
        delayMicroseconds(PRINT_TIME_);

        digitalWrite(PIN_STB4, 1);
        delayMicroseconds((PRINT_TIME + addTime[3] + STB4_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB4, 0);
        delayMicroseconds(PRINT_TIME_);
        // goFront1();

        digitalWrite(PIN_STB5, 1);
        delayMicroseconds((PRINT_TIME + addTime[4] + STB5_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB5, 0);
        delayMicroseconds(PRINT_TIME_);

        digitalWrite(PIN_STB6, 1);
        delayMicroseconds((PRINT_TIME + addTime[5] + STB6_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB6, 0);
        delayMicroseconds(PRINT_TIME_);
        goFront1();
      }
      // delayMicroseconds(PRINT_TIME_);
      // goFront1();
      // delayMicroseconds(PRINT_TIME_);
      // goFront1();
      // delayMicroseconds(PRINT_TIME_);
      // goFront1();
    }

    digitalWrite(PIN_MOTOR_AP, 0);
    digitalWrite(PIN_MOTOR_AM, 0);
    digitalWrite(PIN_MOTOR_BP, 0);
    digitalWrite(PIN_MOTOR_BM, 0);

    clearAddTime();
    clearSTB();
    clearData();
    printDataCount = 0;
    Serial.println("[INFO]打印完成");
    digitalWrite(PIN_VHEN, 0);
    // digitalWrite(PIN_STATUS, 0);
    startBeep();
    delay(100);
    stopBeep();
    delay(50);
    startBeep();
    delay(100);
    stopBeep();
  }
}

void startPrint(uint8_t stb)
{
  if (PaperSta == 1)
  {
    static unsigned char motor_add = 0;
    startBeep();
    delay(700);
    stopBeep();

    Serial.printf("[INFO]正在打印STB%c\n", stb + 0x31);
    Serial.printf("[INFO]共%u行\n", printDataCount / 48);
    digitalWrite(PIN_VHEN, 1);
    // digitalWrite(PIN_STATUS, 1);
    for (uint32_t pointer = 0; pointer < printDataCount; pointer += 48)
    {
      motor_add++;
      if (motor_add != 40)
      {
        delayMicroseconds((PRINT_TIME) * ((double)heat_density / 100));
        goFront1();
      }
      else
      {
        motor_add = 0;
      }
      clearAddTime();
      sendData(printData + pointer);
      switch (stb)
      {
      case 0:
        digitalWrite(PIN_STB1, 1);
        delayMicroseconds((PRINT_TIME + addTime[0] + STB1_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB1, 0);
        delayMicroseconds(PRINT_TIME_);
        break;
      case 1:
        digitalWrite(PIN_STB2, 1);
        delayMicroseconds((PRINT_TIME + addTime[1] + STB2_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB2, 0);
        delayMicroseconds(PRINT_TIME_);
        break;
      case 2:
        digitalWrite(PIN_STB3, 1);
        delayMicroseconds((PRINT_TIME + addTime[2] + STB3_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB3, 0);
        delayMicroseconds(PRINT_TIME_);
        break;
      case 3:
        digitalWrite(PIN_STB4, 1);
        delayMicroseconds((PRINT_TIME + addTime[3] + STB4_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB4, 0);
        delayMicroseconds(PRINT_TIME_);
        break;
      case 4:
        digitalWrite(PIN_STB5, 1);
        delayMicroseconds((PRINT_TIME + addTime[4] + STB5_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB5, 0);
        delayMicroseconds(PRINT_TIME_);
        break;
      case 5:
        digitalWrite(PIN_STB6, 1);
        delayMicroseconds((PRINT_TIME + addTime[5] + STB6_ADDTIME) * ((double)heat_density / 100));
        digitalWrite(PIN_STB6, 0);
        delayMicroseconds(PRINT_TIME_);
        break;
      }
      // goFront(39, 3000);
      goFront(390, 3000);
    }

    digitalWrite(PIN_MOTOR_AP, 0);
    digitalWrite(PIN_MOTOR_AM, 0);
    digitalWrite(PIN_MOTOR_BP, 0);
    digitalWrite(PIN_MOTOR_BM, 0);
    clearAddTime();
    clearSTB();
    clearData();
    digitalWrite(PIN_VHEN, 0);
    // digitalWrite(PIN_STATUS, 0);
    startBeep();
    delay(100);
    stopBeep();
    delay(50);
    startBeep();
    delay(100);
    stopBeep();
  }
}

void clearSTB()
{
  digitalWrite(PIN_STB1, 0);
  digitalWrite(PIN_STB2, 0);
  digitalWrite(PIN_STB3, 0);
  digitalWrite(PIN_STB4, 0);
  digitalWrite(PIN_STB5, 0);
  digitalWrite(PIN_STB6, 0);
}

// 打印头测试
void testPage(uint8_t stb)
{
  Serial.println("开始打印 颜色深度-同时打印点数 测试页\n可根据此页调整加热时间常数");
  uint8_t printchr[8] = {0};
  uint8_t i, j, k;
  uint8_t dots, dotsnow;
  printDataCount = 0;
  for (uint32_t cleardata = 0; cleardata < 102400; ++cleardata)
  {
    printData[cleardata] = 0;
  }
  for (uint8_t dots = 0; dots < 64; dots += 4)
  {
    for (k = 0; k < 5; ++k)
    {
      dotsnow = 0;
      for (i = 0; i < 8; ++i)
      {
        printchr[i] = 0;
      }

      for (i = 0; i < 8; ++i)
      {
        for (j = 0; j < 8; ++j)
        {
          if (dotsnow == dots)
          {
            break;
          }
          dotsnow++;
          printchr[i] |= (0x80 >> j);
        }
      }
      memcpy(printData + printDataCount + 8, printchr, 8);
      memcpy(printData + printDataCount + 16, printchr, 8);
      memcpy(printData + printDataCount + 24, printchr, 8);
      memcpy(printData + printDataCount + 32, printchr, 8);
      memcpy(printData + printDataCount + 40, printchr, 8);
      memcpy(printData + printDataCount + 48, printchr, 8);
      printDataCount += 48;
    }
    printDataCount += 48 * 3;
  }
  startBeep();
  delay(100);
  stopBeep();
  delay(50);
  startBeep();
  delay(200);
  stopBeep();
  delay(50);
  startBeep();
  delay(100);
  stopBeep();
  delay(50);
  startPrint(stb);
  goFront(40 * 4, MOTOR_TIME);
  printDataCount = 0;
}

void testSTB()
{
  Serial.println("开始打印打印头选通引脚(Strobe)测试页\n顺序:开头  1  2  3  4  5  6");
  for (uint32_t cleardata = 0; cleardata < 48 * 5; ++cleardata)
  {
    printData[cleardata] = 0xff;
  }
  printDataCount = 48 * 5;
  startPrint(0);
  startPrint(1);
  startPrint(2);
  startPrint(3);
  startPrint(4);
  startPrint(5);
  startBeep();
  delay(200);
  stopBeep();
  delay(50);
  startBeep();
  delay(200);
  stopBeep();
  delay(50);
  startBeep();
  delay(100);
  stopBeep();
  delay(50);
  goFront(40 * 4, MOTOR_TIME);
  printDataCount = 0;
}
