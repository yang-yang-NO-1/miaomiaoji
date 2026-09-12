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

#define P2_MOTOR_TIME 3000

// Shared runtime state and module APIs are declared in include/*.h.


#define START_BYTE 0x02             // 开始字节
#define END_BYTE 0x03               // 结束字节
#define PRINTER_SN "P2B02004087288" // 原工程保留的P2设备编号
#define PRINTER_NAME "P2"            // APP列表中存在且支持标签打印的型号
#define COUNTRY_NAME "CN" // 国家的名字
const char CMD_42_DATA[] PROGMEM = "BK3432";
uint8_t CMD_7F_DATA[] PROGMEM = {0x76, 0x33, 0x2e, 0x33, 0x38, 0x2e, 0x31, 0x39, 0, 0, 0, 0};
uint8_t CMD_81_DATA[] PROGMEM = {0x48, 0x4d, 0x45, 0x32, 0x33, 0x30, 0x5f, 0x50, 0x32, 0, 0, 0, 0, 0, 0, 0};
// uint8_t PRINTER_VERSION[] PROGMEM = {0x08, 0x01, 0x01};     //打印机版本1.1.8
uint8_t PRINTER_VERSION[] PROGMEM = {0x01, 0x00, 0x02}; // 打印机版本2.0.1
uint8_t *CMD_40_DATA PROGMEM = {0x00};
uint16_t max_gap_length = 100;
uint8_t paper_type = 0;
bool print_job_finished = true;

#define P2_ROW_BYTES 72
#define P1_ROW_BYTES 48

// 实测APP对29/43/53 mm标签均发送FEED_TO_HEAD=3840，标签长度实际编码在
// 位图行数中。当前机芯每打印1行走4个半步、约8行/mm，因此约32个半步/mm。
// 29 mm样本：转换后114行，已走456步；补472步后总计928步，正好29 mm。
#define MOTOR_HALF_STEPS_PER_MM 32UL
#define MOTOR_HALF_STEPS_PER_PRINT_ROW 4UL
#define LABEL_NON_BITMAP_STEPS 472UL
#define LABEL_DEFAULT_PITCH_MM 29UL

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

#define GET_PRINT_PROGRESS 52
#define SENT_PRINT_PROGRESS 53
#define GET_PRINT_FINISHED 54
#define SENT_PRINT_FINISHED 55
#define GET_DEVICE_CAPABILITY 60
#define SENT_DEVICE_CAPABILITY 61

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
uint8_t c;
uint8_t dataPack_read[2048];
enum PaperangRxState : uint8_t
{
  RX_WAIT_START,
  RX_TYPE,
  RX_INDEX,
  RX_LENGTH_LOW,
  RX_LENGTH_HIGH,
  RX_PAYLOAD,
  RX_CRC,
  RX_END
};


PaperangRxState rx_state = RX_WAIT_START;
uint16_t rx_payload_pos = 0;
uint8_t rx_crc_pos = 0;
uint32_t rx_print_start_count = 0;
// volatile uint8_t cacheLock = 0;
uint16_t paperang_read_le16(uint16_t fallback)
{
  if (packHeader.dataLen == 0)
  {
    return fallback;
  }
  uint16_t value = dataPack_read[0];
  if (packHeader.dataLen >= 2)
  {
    value |= ((uint16_t)dataPack_read[1] << 8);
  }
  return value;
}

// P2图像是576点/行、300dpi；当前打印头是384点/行、约200dpi。
// 横向和纵向同时缩放为2/3，避免原工程注释中记录的拉长和交错重复。
bool convert_p2_bitmap_to_p1()
{
  if (printDataCount == 0 || (printDataCount % P2_ROW_BYTES) != 0)
  {
    Serial.printf("[P2 SCALE] 数据长度错误: %u（应为72的倍数）\n", printDataCount);
    return false;
  }

  uint32_t source_rows = printDataCount / P2_ROW_BYTES;
  uint32_t target_rows = (source_rows * 2 + 1) / 3;
  uint8_t source_row[P2_ROW_BYTES];

  for (uint32_t target_y = 0; target_y < target_rows; ++target_y)
  {
    uint32_t source_y = (target_y * 3 + 1) / 2;
    if (source_y >= source_rows)
    {
      source_y = source_rows - 1;
    }

    memcpy(source_row, printData + source_y * P2_ROW_BYTES, P2_ROW_BYTES);
    uint8_t *target_row = printData + target_y * P1_ROW_BYTES;
    memset(target_row, 0, P1_ROW_BYTES);

    for (uint16_t target_x = 0; target_x < 384; ++target_x)
    {
      uint16_t source_x = (target_x * 3 + 1) / 2;
      if (source_row[source_x >> 3] & (0x80 >> (source_x & 7)))
      {
        target_row[target_x >> 3] |= (0x80 >> (target_x & 7));
      }
    }
  }

  printDataCount = target_rows * P1_ROW_BYTES;
  Serial.printf("[P2 SCALE] %u行 -> %u行, 输出=%u字节\n",
                source_rows, target_rows, printDataCount);
  return true;
}

uint32_t bitmap_set_bit_count(const uint8_t *data, uint32_t len)
{
  uint32_t count = 0;
  for (uint32_t i = 0; i < len; ++i)
  {
    count += __builtin_popcount((unsigned int)data[i]);
  }
  return count;
}

// 普通打印以 FEED_LINE 结束，标签打印则以 FEED_TO_HEAD_LINE 结束。
// 两条路径必须先把缓存中的 P2 点阵转换并送往当前 384 点打印头。
bool print_buffered_p2_job(const char *trigger, uint32_t *printed_rows)
{
  *printed_rows = 0;
  if (printDataCount == 0)
  {
    Serial.printf("[BITMAP] %s 无缓存数据\n", trigger);
    return false;
  }

  uint32_t source_bytes = printDataCount;
  uint32_t source_bits = bitmap_set_bit_count(printData, source_bytes);
  Serial.printf("[BITMAP] %s 输入=%u字节, 黑点=%u\n",
                trigger, source_bytes, source_bits);

  if (!convert_p2_bitmap_to_p1())
  {
    return false;
  }

  const uint32_t requested_rows = printDataCount / P1_ROW_BYTES;
  uint32_t output_bits = bitmap_set_bit_count(printData, printDataCount);
  Serial.printf("[BITMAP] 输出=%u行, 黑点=%u\n", requested_rows, output_bits);

  const uint32_t completed_rows = startPrint();
  *printed_rows = completed_rows;
  const bool ok = (completed_rows == requested_rows);
  print_job_finished = true;

  if (!ok)
  {
    Serial.printf("[BITMAP] 打印中止: 完成=%u/%u行\n", completed_rows, requested_rows);
    return false;
  }
  return true;
}

void paperang_process_data()
{
  uint32_t tmp32;
  uint16_t feed_lines;
  Serial.printf("[BT RX] CMD=0x%02X LEN=%u\n", packHeader.packType, packHeader.dataLen);
  switch (packHeader.packType)
  {
  case PRINT_DATA:
    print_job_finished = false;
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
    paperang_send_msg(CMD_43, reinterpret_cast<const uint8_t *>(CMD_42_DATA), strlen(CMD_42_DATA));
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
      PowerOFFTime = static_cast<uint32_t>(9999999999ULL);
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
    HeatTemp(true); // GET_TEMP时强制输出一次实时温度日志
    paperang_send_msg(SENT_TEMP, &head_temp, 1);
    break;
  case SET_MAX_GAP_LENGTH:
    max_gap_length = paperang_read_le16(max_gap_length);
    Serial.printf("[LABEL] MAX_GAP=%u\n", max_gap_length);
    break;
  case GET_MAX_GAP_LENGTH:
    paperang_send_msg(SENT_MAX_GAP_LENGTH, (uint8_t *)&max_gap_length, 2);
    break;
  case SET_PAPER_TYPE:
    if (packHeader.dataLen > 0)
    {
      paper_type = dataPack_read[0];
    }
    Serial.printf("[LABEL] PAPER_TYPE=%u\n", paper_type);
    break;
  case GET_PAPER_TYPE:
    paperang_send_msg(SENT_PAPER_TYPE, &paper_type, 1);
    break;
  case FEED_TO_HEAD_LINE:
    feed_lines = paperang_read_le16(0);
    Serial.printf("[LABEL] FEED_TO_HEAD=%u\n", feed_lines);
    {
      uint32_t source_rows = printDataCount / P2_ROW_BYTES;
      uint32_t printed_rows = 0;
      bool printed = print_buffered_p2_job("CMD_0x21", &printed_rows);
      if (!printed)
      {
        Serial.println("[LABEL] 打印未完成，安全保护已触发，跳过后续定位走纸");
        printDataCount = 0;
        break;
      }

      // APP 6.2.80对不同标签长度均发送3840；位图行数才随纸张长度变化。
      // 位图覆盖标签的可打印区，29 mm实测还需固定补472步非位图区。
      uint32_t printed_motor_steps = printed_rows * MOTOR_HALF_STEPS_PER_PRINT_ROW;
      uint32_t target_motor_steps = printed_motor_steps + LABEL_NON_BITMAP_STEPS;
      uint32_t remaining_motor_steps =
          target_motor_steps > printed_motor_steps ? target_motor_steps - printed_motor_steps : 0;
      uint32_t pitch_tenth_mm =
          (target_motor_steps * 10 + MOTOR_HALF_STEPS_PER_MM / 2) / MOTOR_HALF_STEPS_PER_MM;
      Serial.printf("[LABEL] APP寻标参数=%u, 原始行=%u, 输出行=%u, 识别节距=%u.%umm, 目标步数=%u, 已打印步数=%u, 补走=%u\n",
                    feed_lines, source_rows, printed_rows,
                    pitch_tenth_mm / 10, pitch_tenth_mm % 10,
                    target_motor_steps,
                    printed_motor_steps, remaining_motor_steps);
      if (remaining_motor_steps > 0)
      {
        goFront(remaining_motor_steps, P2_MOTOR_TIME);
      }
      printDataCount = 0;
    }
    break;
  case GET_PRINT_PROGRESS:
    tmp32 = printDataCount / P2_ROW_BYTES;
    paperang_send_msg(SENT_PRINT_PROGRESS, (uint8_t *)&tmp32, 4);
    break;
  case GET_PRINT_FINISHED:
  {
    uint8_t finished = print_job_finished ? 1 : 0;
    paperang_send_msg(SENT_PRINT_FINISHED, &finished, 1);
    break;
  }
  case GET_DEVICE_CAPABILITY:
  {
    uint32_t capability = 2;
    paperang_send_msg(SENT_DEVICE_CAPABILITY, (uint8_t *)&capability, 4);
    break;
  }
  // 进料线
  case FEED_LINE:
    feed_lines = paperang_read_le16(Finish_Out);
    Serial.printf("[PRINT] P2_FEED_LINE=%u\n", feed_lines);
    if (printDataCount != 0)
    {
      uint32_t printed_rows = 0;
      if (print_buffered_p2_job("CMD_0x1A", &printed_rows))
      {
        (void)printed_rows;
        if (IsPaperPresent())
        {
          goFront(Finish_Out, P2_MOTOR_TIME); // 打印完毕出纸长度，单位：点，8点/mm。
        }
      }
      else
      {
        Serial.println("[PRINT] 打印未完成，跳过打印后走纸");
      }
    }
    printDataCount = 0;
    break;
  default:
    Serial.printf("[BT] ACK_ONLY CMD=0x%02X\n", packHeader.packType);
    break;
  }
  paperang_send_ack(packHeader.packType);
}

void paperang_rx_reset()
{
  rx_state = RX_WAIT_START;
  rx_payload_pos = 0;
  rx_crc_pos = 0;
  packHeader.packType = 0;
  packHeader.packIndex = 0;
  packHeader.dataLen = 0;
}

void paperang_rx_start()
{
  rx_print_start_count = printDataCount;
  rx_payload_pos = 0;
  rx_crc_pos = 0;
  packHeader.packType = 0;
  packHeader.packIndex = 0;
  packHeader.dataLen = 0;
  rx_state = RX_TYPE;
}

void paperang_rx_abort(const char *reason, uint8_t value)
{
  // 如果坏包属于打印数据，丢弃这个包已经写入缓存的部分，避免行宽错位。
  if (packHeader.packType == PRINT_DATA && printDataCount > rx_print_start_count)
  {
    printDataCount = rx_print_start_count;
  }
  Serial.printf("[BT RX ERROR] %s state=%u value=0x%02X\n",
                reason, (unsigned int)rx_state, value);
  paperang_rx_reset();

  // 当前字节可能已经是下一个包的起始符，直接用于重新同步。
  if (value == START_BYTE)
  {
    paperang_rx_start();
  }
}

void paperang_rx_byte(uint8_t value)
{
  switch (rx_state)
  {
  case RX_WAIT_START:
    if (value == START_BYTE)
    {
      paperang_rx_start();
    }
    break;

  case RX_TYPE:
    packHeader.packType = value;
    rx_state = RX_INDEX;
    break;

  case RX_INDEX:
    packHeader.packIndex = value;
    rx_state = RX_LENGTH_LOW;
    break;

  case RX_LENGTH_LOW:
    packHeader.dataLen = value;
    rx_state = RX_LENGTH_HIGH;
    break;

  case RX_LENGTH_HIGH:
    packHeader.dataLen |= ((uint16_t)value << 8);
    if (packHeader.packType != PRINT_DATA && packHeader.dataLen > sizeof(dataPack_read))
    {
      paperang_rx_abort("payload too large", value);
      break;
    }
    if (packHeader.packType == PRINT_DATA &&
        printDataCount + packHeader.dataLen > PRINT_DATA_CAPACITY)
    {
      paperang_rx_abort("print buffer full", value);
      break;
    }
    rx_state = packHeader.dataLen == 0 ? RX_CRC : RX_PAYLOAD;
    break;

  case RX_PAYLOAD:
    if (packHeader.packType == PRINT_DATA)
    {
      printData[printDataCount++] = value;
    }
    else
    {
      dataPack_read[rx_payload_pos] = value;
    }
    ++rx_payload_pos;
    if (rx_payload_pos == packHeader.dataLen)
    {
      rx_crc_pos = 0;
      rx_state = RX_CRC;
    }
    break;

  case RX_CRC:
    // 保持原工程行为：接收但不校验4字节CRC。
    ++rx_crc_pos;
    if (rx_crc_pos == 4)
    {
      rx_state = RX_END;
    }
    break;

  case RX_END:
    if (value == END_BYTE)
    {
      paperang_process_data();
      paperang_rx_reset();
    }
    else
    {
      paperang_rx_abort("missing end byte", value);
    }
    break;
  }
}
void paperang_core0()
{
  // Reserved for compatibility with the original project.
}

void paperang_bt_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  (void)param;
  if (event == ESP_SPP_SRV_OPEN_EVT)
  {
    Serial.println("[BT] CONNECTED");
  }
  else if (event == ESP_SPP_CLOSE_EVT)
  {
    Serial.println("[BT] DISCONNECTED");
  }
}

void paperang_app()
{
  SerialBT.register_callback(paperang_bt_callback);
  SerialBT.begin("MaoPaperang"); // 蓝牙设备名称
  // 重新设置class of device
  esp_bt_cod_t cod;
  cod.major = 6;               // 主设备类型
  cod.minor = 0b100000;        // 次设备类型
  cod.service = 0b00000100000; // 服务类型
  esp_bt_gap_set_cod(cod, ESP_BT_INIT_COD);
  crc32.init(0x35769521);
  paperang_rx_reset();
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
    while (SerialBT.available())
    {
      c = (uint8_t)SerialBT.read();
      paperang_rx_byte(c);
    }
  }
}
