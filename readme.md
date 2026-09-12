# MaoPaperang P2 Label V6

ESP32-WROVER 热敏标签打印机固件。当前版本已匹配 Mao 喵喵机 V2 硬件，并完成温度检测、过温保护、缺纸检测与打印中安全停止的实机验证。

## 当前已验证功能

- Paperang P2 蓝牙协议兼容与标签数据接收
- P2 576 点位图转换为 384 点打印头数据
- 标签打印与固定补走定位
- GPIO35 打印头 NTC 温度检测
- 60°C 过温暂停，55°C 恢复后续打
- GPIO34 缺纸检测
- 打印中缺纸立即终止当前任务，重新装纸后等待 APP 重发
- 缺纸/过温时关闭打印头加热电源

## 工程结构

```text
MaoPaperang_P2_Label_V6_Organized/
├─ include/
│  ├─ main.h                 # 聚合入口，仅负责包含各模块头文件
│  ├─ HardwarePins.h         # GPIO 与蜂鸣器定义
│  ├─ PrinterConfig.h        # 打印、温度、缺纸参数
│  ├─ AppState.h             # 跨模块运行状态声明
│  ├─ Sensors.h              # 温度/缺纸/电池/按键接口
│  ├─ Printer.h              # 电机、SPI、打印接口
│  └─ PaperangProtocol.h     # Paperang 蓝牙协议入口
├─ src/
│  ├─ main.cpp               # 硬件初始化与程序入口
│  ├─ AppState.cpp           # 全局运行状态的唯一实现位置
│  ├─ Sensors.cpp            # 温度、缺纸、电池、按键、安全保护
│  ├─ Printer.cpp            # 打印头、电机、STB 与测试页
│  ├─ Paperang.cpp           # 蓝牙协议、P2数据解析、标签逻辑
│  ├─ Arduino_CRC32.*        # CRC32
│  └─ crc.*                  # 原工程CRC代码
├─ lib/BluetoothSerial/      # 项目自带 BluetoothSerial
├─ docs/
│  ├─ CODE_STRUCTURE.md      # 模块说明与维护边界
│  ├─ P2_LABEL_TEST_README.md
│  ├─ SENSOR_SAFETY_CHANGELOG.md
│  ├─ ORIGINAL_README.md
│  ├─ hardware/              # 规格书、原理图JSON、PCB JSON
│  └─ images/                # 原README图片
└─ platformio.ini
```

## 编译

默认环境保持原工程配置：

```ini
[platformio]
default_envs = esp-wrover-kit
```

在 VS Code + PlatformIO 中直接执行 **Build** 或 **Upload** 即可。

## 维护原则

本次整理以“不改变已实机验证的打印行为”为第一原则。协议中若干历史兼容写法虽然可以进一步现代化，但暂未随结构整理一起修改，避免影响 Paperang APP 兼容性。需要继续清理时建议一次只改一个模块，并保留串口回归日志。
