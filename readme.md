# 此工程时使用esp32实现的喵喵机
来源于立创开源广场的樱猫不知味<https://oshwhub.com/SakuraNeko/mao-re-min-da-yin-ji>，原工程为arduino平台代码，移植到pio平台，最初工程来源于<https://github.com/lxydiy/ESP32-Paperang-Emulator>
## esp32 wrover在PIO中使用ram
![](readme/20230422212209.png)
## platform.ini文件参考示例<https://docs.platformio.org/en/latest/platforms/espressif32.html>
## 首次连接APP及正常打印的串口输出
![](readme/20230424192440.png)
## 通信协议备注
![](readme/20230425121752.png)
## 增加功能
1.电池电量（ADC不准）
2.打印头温度检测（ADC不准）
3.缺纸检测(可用)，缺纸时会滴滴（两声）叫