#include "main.h"
#include "EEPROM.h"
#include <BluetoothSerial.h>
#include "Arduino_CRC32.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "esp_task_wdt.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define MOTOR_TIME 3000

extern uint32_t printDataCount;
uint8_t PRINTER_BATTERY = 90;
uint32_t power_down_time = 3600;
uint32_t PowerOFFTime = 9999999999;
uint8_t head_temp = 23;
uint32_t printDataCount = 0;
uint8_t *printData;
int PaperSta = 1;
uint32_t PowerONTime = 0;
uint16_t temp_adc;
uint16_t adc;
uint32_t adcc;
int PwrOFFtime = 1;
float tmpAddTime = 0;
float addTime[6] = {0};
extern void BatteryPower();
extern void startPrint();
extern void goFront(uint32_t steps, uint16_t wait);
extern void testPage();
extern void testPage(uint8_t stb);
extern void testSTB();
extern void ButtonRun();
extern void STAPowerOFF();
// 获取打印头温度
void HeatTemp();
extern void PaperCheck();

#define START_BYTE 0x02             // 开始字节
#define END_BYTE 0x03               // 结束字节
#define PRINTER_SN "P1001705253855" // P1打印机设备编号
// #define PRINTER_SN "P2B02004087288"      //P2打印机设备编号,设置为P2会导致无法正常打印：打印边长，交错重复
#define PRINTER_NAME "P1" // P1打印机名称
// #define PRINTER_NAME "P2"                //P2打印机名称
#define COUNTRY_NAME "CN" // 国家的名字
char *CMD_42_DATA PROGMEM = "BK3432";
uint8_t CMD_7F_DATA[] PROGMEM = {0x76, 0x33, 0x2e, 0x33, 0x38, 0x2e, 0x31, 0x39, 0, 0, 0, 0};
uint8_t CMD_81_DATA[] PROGMEM = {0x48, 0x4d, 0x45, 0x32, 0x33, 0x30, 0x5f, 0x50, 0x32, 0, 0, 0, 0, 0, 0, 0};
// uint8_t PRINTER_VERSION[] PROGMEM = {0x08, 0x01, 0x01};     //打印机版本1.1.8
uint8_t PRINTER_VERSION[] PROGMEM = {0x01, 0x00, 0x02}; // 打印机版本2.0.1
uint8_t *CMD_40_DATA PROGMEM = {0x00};
// uint8_t heat_density = 16;                                  //热密度
uint8_t heat_density = 64; // 热密度

#define PRINT_DATA 0            // 打印数据
#define PRINT_DATA_COMPRESS 1   // 打印数据压缩
#define FIRMWARE_DATA 2         // 固件数据
#define USB_UPDATE_FIRMWARE 3   // USB更新固件
#define GET_VERSION 4           // 获取版本
#define SENT_VERSION 5          // 发送版本
#define GET_MODEL 6             // 获取模式
#define SENT_MODEL 7            // 发送模式
#define GET_BT_MAC 8            // 获取蓝牙MAC
#define SENT_BT_MAC 9           // 发送蓝牙MAC
#define GET_SN 10               // 获取SN码
#define SENT_SN 11              // 发送SN码
#define GET_STATUS 12           // 获取状态
#define SENT_STATUS 13          // 发送状态
#define GET_VOLTAGE 14          // 获取电压
#define SENT_VOLTAGE 15         // 发送电压
#define GET_BAT_STATUS 16       // 获取蓝牙状态
#define SENT_BAT_STATUS 17      // 发送蓝牙状态
#define GET_TEMP 18             // 获取打印头温度
#define SENT_TEMP 19            // 发送打印头温度
#define SET_FACTORY_STATUS 20   // 设置出厂状态
#define GET_FACTORY_STATUS 21   // 获取出厂状态
#define SENT_FACTORY_STATUS 22  // 发送出厂状态
#define SENT_BT_STATUS 23       // 发送蓝牙状态
#define SET_CRC_KEY 24          // 设置CRC密钥
#define SET_HEAT_DENSITY 25     // 设定热密度
#define FEED_LINE 26            // 进料线
#define PRINT_TEST_PAGE 27      // 打印测试页
#define GET_HEAT_DENSITY 28     // 获取热密度
#define SENT_HEAT_DENSITY 29    // 发送热密度
#define SET_POWER_DOWN_TIME 30  // 设定关机时间
#define GET_POWER_DOWN_TIME 31  // 获取断电时间
#define SENT_POWER_DOWN_TIME 32 // 发送断电时间
#define FEED_TO_HEAD_LINE 33    // 送至标题行
#define PRINT_DEFAULT_PARA 34   // 打印默认参数
#define GET_BOARD_VERSION 35    // 获取板子版本
#define SENT_BOARD_VERSION 36   // 发送板子版本
#define GET_HW_INFO 37          // 获取硬件信息
#define SENT_HW_INFO 38         // 发送硬件信息
#define SET_MAX_GAP_LENGTH 39   // 设定最大间隙长度
#define GET_MAX_GAP_LENGTH 40   // 获取最大间隙长度
#define SENT_MAX_GAP_LENGTH 41  // 发送最大间隙长度
#define GET_PAPER_TYPE 42       // 获取纸张类型
#define SENT_PAPER_TYPE 43      // 发送纸张类型
#define SET_PAPER_TYPE 44       // 设置纸张类型
#define GET_COUNTRY_NAME 45     // 获取国家名称
#define SENT_COUNTRY_NAME 46    // 发送国家名称
#define DISCONNECT_BT 47        // 断开蓝牙
#define GET_DEV_NAME 48         // 获取设备名称
#define SENT_DEV_NAME 49        // 发送设备名称
#define CMD_39 57
#define CMD_40 64
#define CMD_41 65
#define CMD_42 66
#define CMD_43 67
#define CMD_7F 127
#define CMD_80 128
#define CMD_81 129
#define CMD_82 130

BluetoothSerial SerialBT;
Arduino_CRC32 crc32;
uint8_t dataPack[512];
uint32_t dataPack_len;
uint32_t crc32_result;
void paperang_send(void)
{
  SerialBT.write(dataPack, dataPack_len);
}

void paperang_send_ack(uint8_t type)
{
  uint8_t ackcrc = 0;
  dataPack[0] = START_BYTE;
  dataPack[1] = type;
  dataPack[2] = 0x00;
  dataPack[3] = 0x01;
  dataPack[4] = 0x00;
  dataPack[5] = 0x00;
  crc32_result = crc32.calc(&ackcrc, 1);
  memcpy(dataPack + 6, (uint8_t *)&crc32_result, 4);
  dataPack[10] = END_BYTE;
  dataPack_len = 11;
  paperang_send();
}

void paperang_send_msg(uint8_t type, const uint8_t *dat, uint16_t len)
{
  dataPack[0] = START_BYTE;
  dataPack[1] = type;
  dataPack[2] = 0x00;
  memcpy(dataPack + 3, (uint8_t *)&len, 2);
  memcpy(dataPack + 5, dat, len);
  dataPack_len = 5 + len;
  crc32_result = crc32.calc(dat, len);
  memcpy(dataPack + dataPack_len, (uint8_t *)&crc32_result, 4);
  dataPack[dataPack_len + 4] = END_BYTE;
  dataPack_len += 5;
  paperang_send();
}

struct
{
  uint8_t packType;
  uint8_t packIndex;
  uint16_t dataLen;
} packHeader;
uint8_t gotStartByte = 0;
uint8_t c;
uint16_t readpos = 0;
uint8_t dataPack_read[2048];
// #define PRINT_DATA_CACHE_SIZE 10240
// uint8_t printDataCache[PRINT_DATA_CACHE_SIZE + 1024];
// volatile uint16_t cacheOverflowed = 0;
// volatile uint16_t printDataCacheHead = 0;
// volatile uint16_t printDataCacheFoot = 0;
uint16_t dataPack_read_pos = 0;
// volatile uint8_t cacheLock = 0;
void paperang_process_data()
{
  uint32_t tmp32;
  switch (packHeader.packType)
  {
  case PRINT_DATA:
    /*
      //while (cacheLock);
      //cacheLock = 1;
      memcpy(printDataCache + printDataCacheHead, dataPack_read, packHeader.dataLen);
      printDataCacheHead += packHeader.dataLen;
      if (printDataCacheHead >= PRINT_DATA_CACHE_SIZE)
      {
        printDataCacheHead = 0;
        cacheOverflowed = printDataCacheHead - PRINT_DATA_CACHE_SIZE;
      }
      //cacheLock = 0;*/
    return;

  // 设置CRC密钥
  case SET_CRC_KEY:
    tmp32 = dataPack_read[0] << 24 + dataPack_read[1] << 16 + dataPack_read[2] << 8 + dataPack_read[3];
    crc32.init(tmp32);
    break;
  // 获取版本信息并发送
  case GET_VERSION:
    paperang_send_msg(SENT_VERSION, PRINTER_VERSION, 3);
    break;
  // 获取设备名称并发送
  case GET_DEV_NAME:
    paperang_send_msg(SENT_DEV_NAME, (uint8_t *)PRINTER_NAME, 2);
    break;
  // 获取SN码并发送
  case GET_SN:
    paperang_send_msg(SENT_SN, (uint8_t *)PRINTER_SN, strlen((char *)PRINTER_SN));
  // 获取电池电量并发送
  case GET_BAT_STATUS:
    BatteryPower();
    paperang_send_msg(SENT_BAT_STATUS, &PRINTER_BATTERY, 1);
    break;
  // 获取国家名称并发送
  case GET_COUNTRY_NAME:
    paperang_send_msg(SENT_COUNTRY_NAME, (uint8_t *)COUNTRY_NAME, 2);
    break;
  case CMD_42:
    paperang_send_msg(CMD_43, (uint8_t *)CMD_42_DATA, strlen(CMD_42_DATA));
    break;
  case CMD_7F:
    paperang_send_msg(CMD_80, CMD_7F_DATA, 12);
    break;
  case CMD_81:
    paperang_send_msg(CMD_82, CMD_81_DATA, 16);
    break;
  case CMD_40:
    paperang_send_msg(CMD_41, CMD_40_DATA, 1);
    break;
  // 设置断电时间
  case SET_POWER_DOWN_TIME:
    power_down_time = dataPack_read[0] << 8 + dataPack_read[1];
    if (power_down_time == 90112)
    {
      PowerOFFTime = 600000;
      Serial.print("自动关机时间设置为：10分钟");
    }
    else if (power_down_time == 262144)
    {
      PowerOFFTime = 1800000;
      Serial.print("自动关机时间设置为：30分钟");
    }
    else if (power_down_time == 67108864)
    {
      PowerOFFTime = 3600000;
      Serial.print("自动关机时间设置为：1小时");
    }
    else if (power_down_time == 12582912)
    {
      PowerOFFTime = 10800000;
      Serial.print("自动关机时间设置为：3小时");
    }
    else if (power_down_time == 1310720)
    {
      PowerOFFTime = 18000000;
      Serial.print("自动关机时间设置为：5小时");
    }
    else if (power_down_time == 12582912)
    {
      PowerOFFTime = 36000000;
      Serial.print("自动关机时间设置为：10小时");
    }
    else if (power_down_time == 1310720)
    {
      PowerOFFTime = 9999999999;
      Serial.print("关闭自动关机");
    }
    EEPROM.write(PwrOFFtime, PowerOFFTime);
    break;
  // 发送断电时间并发送
  case GET_POWER_DOWN_TIME:
    paperang_send_msg(SENT_POWER_DOWN_TIME, (uint8_t *)&power_down_time, 2);
    break;
  // 设置打印密度
  case SET_HEAT_DENSITY:
    heat_density = dataPack_read[0];
    break;
  // 获取打印密度并发送
  case GET_HEAT_DENSITY:
    paperang_send_msg(SENT_HEAT_DENSITY, &heat_density, 1);
    break;
  // 获取打印头温度
  case GET_TEMP:
    HeatTemp();// 获取温度值
    paperang_send_msg(SENT_TEMP, &head_temp, 1);
    break;
  // 进料线
  case FEED_LINE:
    if (printDataCount / 48 != 0)
    {
      startPrint();
      goFront(Finish_Out, MOTOR_TIME); // 打印完毕出纸长度，单位：点，8点/mm。
    }
    printDataCount = 0;
    break;
  default:
    break;
  }
  paperang_send_ack(packHeader.packType);
}
/*
  void paperang_copy(void *pvParameters)
  {
  (void) pvParameters;
  esp_task_wdt_init(1024000, false);
  uint32_t newDataCount;
  uint16_t tmp16;
  while (1)
  {
    if (printDataCacheHead != printDataCacheFoot)
    {
      //while (cacheLock);
      //cacheLock = 1;
      if (cacheOverflowed)
      {
        tmp16 = printDataCacheFoot;
        newDataCount = PRINT_DATA_CACHE_SIZE - printDataCacheFoot + cacheOverflowed;
        printDataCacheFoot = 0;
        cacheOverflowed = 0;
        memcpy(printData + printDataCount, printDataCache + tmp16, newDataCount);
        printDataCount += newDataCount;
        tmp16 = printDataCacheHead;
        memcpy(printData + printDataCount, printDataCache, tmp16);
        printDataCount += tmp16;
      }
      else
      {
        newDataCount = printDataCacheHead - printDataCacheFoot;
        tmp16 = printDataCacheFoot;
        printDataCacheFoot = printDataCacheHead;
        memcpy(printData + printDataCount, printDataCache + tmp16, newDataCount);
        printDataCount += newDataCount;
      }
      //cacheLock = 0;
    }
    //vTaskDelay(20);
  }
  }
*/
void paperang_core0()
{
  /*
    xTaskCreatePinnedToCore(
    paperang_copy
    ,  "Paperang_copy"   // A name just for humans
    ,  4096
    ,  NULL
    ,  0  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL
    ,  0);
  */
}

void paperang_app()
{
  uint16_t i = 0;
  SerialBT.begin("MaoPaperang"); // 蓝牙设备名称
  // 重新设置class of device
  esp_bt_cod_t cod;
  cod.major = 6;               // 主设备类型
  cod.minor = 0b100000;        // 次设备类型
  cod.service = 0b00000100000; // 服务类型
  esp_bt_gap_set_cod(cod, ESP_BT_INIT_COD);
  crc32.init(0x35769521);
  packHeader.dataLen = 0;
  Serial.println();
  Serial.println("3秒内输入要测试的STB序号开始打印测试页（1-6）: ");
  delay(3000);
  if (Serial.available())
  {
    char chr = Serial.read();
    if (chr >= '1' && chr <= '6')
    {
      chr -= 0x30;
      testPage((uint8_t)chr - 1);
    }
    else if (chr == 'a') // 确认STB位置
    {
      testSTB();
    }
    else if (chr == 'A') // 确认STB位置
    {
      testPage(0);
      testPage(1);
      testPage(2);
      testPage(3);
      testPage(4);
      testPage(5);
    }
    Serial.flush();
  }
  while (1)
  {
    ButtonRun();
    STAPowerOFF();
    // HeatTemp();
    PaperCheck();
    // BatteryPower();
    if (SerialBT.available())
    {
      c = SerialBT.read();// SerialBT.read()每次接受8bit数据
      if (c == START_BYTE && gotStartByte == 0)
      {
        gotStartByte = 1;
        readpos = 0;
      }
      else if (readpos == 1)
      {
        packHeader.packType = c;
        // Serial.println(packHeader.packType);
        packHeader.packIndex = SerialBT.read();
        packHeader.dataLen = SerialBT.read();
        packHeader.dataLen += SerialBT.read() << 8;
        // Serial.println(packHeader.dataLen);
      }
      else if (readpos == 2 && packHeader.dataLen != 0 && packHeader.dataLen < 2048)
      {
        i = packHeader.dataLen - 1;
        if (packHeader.packType == PRINT_DATA)
        {
          printData[printDataCount++] = c;// 蓝牙收到的打印数据
          while (i)
          {
            while (SerialBT.available() == 0)
              ;
            printData[printDataCount++] = SerialBT.read();
            --i;
          }
        }
        else
        {
          dataPack_read[dataPack_read_pos++] = c;// 收到的其他信息
          while (i)
          {
            dataPack_read[dataPack_read_pos++] = SerialBT.read();
            --i;
          }
        }
      }
      else if (readpos < 7)
      {
        ;
      }
      else if (c == END_BYTE && readpos == 7)
      {
        paperang_process_data();// 收到数据结束标志后，下一步进程
        gotStartByte = 0;
        dataPack_read_pos = 0;
        readpos = 0;
        packHeader.dataLen = 0;
      }
      else
      {
        Serial.println("ERROR");
        gotStartByte = 0;
        dataPack_read_pos = 0;
        readpos = 0;
        packHeader.dataLen = 0;
      }
      if (gotStartByte == 1)
        ++readpos;
    }
  }
}
