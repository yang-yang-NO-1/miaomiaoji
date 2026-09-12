# MaoPaperang P2 Label V6 - Sensor Safety Patch

本补丁基于用户提供的 V6 源码增加：

- JX-2R-01 30kΩ/B3950 热敏电阻实际温度换算；
- GPIO35 温度采样与异常检测；
- 60°C停热 / 55°C恢复的打印头过温保护；
- GPIO34 缺纸检测、40ms消抖、状态变化提示；
- 打印前与打印中的缺纸/温度联锁；
- 安全中止后禁止标签定位补走与普通打印尾部走纸；
- GET_TEMP 返回实时温度，不再固定23°C；
- 原缺纸代码的循环重复蜂鸣问题修复。

关键可调参数均集中在 `src/main.h`：

- `TEMP_PULLDOWN_OHM`
- `HEAD_TEMP_STOP_C`
- `HEAD_TEMP_RESUME_C`
- `TEMP_CHECK_EVERY_ROWS`
- `PAPER_PRESENT_LEVEL`
- `PAPER_DEBOUNCE_MS`

硬件默认假设：

- TM：3.3V -> 机芯内部NTC -> TM -> R10(10kΩ) -> GND；TM经R11(1kΩ)串联到GPIO35/TEMP；
- 温度公式已按实际下拉分压方向修正为 `Rntc = R10 * (4095-ADC) / ADC`；
- PHE：机芯缺纸传感器模拟输出 -> LM393反相输入；
- PTEST：LM393开集电极输出 -> GPIO34，R20=10kΩ上拉3.3V；有纸低、缺纸高。


## 2026-09-12 compile compatibility fix
- Arduino-ESP32 1.0.6 / GCC 5.2: use `std::isfinite()` instead of unqualified `isfinite()`.
- Rename the Paperang.cpp local motor timing macro to `P2_MOTOR_TIME` so it no longer conflicts with `MOTOR_TIME` from `main.h`; behavior remains 3000 us in the P2 protocol path.


## 2026-09-12 final safety validation
- Hardware validation passed for GPIO34 paper detection and GPIO35 NTC temperature measurement.
- Mid-print paper-out now aborts and discards the current print job; label post-feed is skipped. Reinserted paper waits for the app to resend a complete job.
- Over-temperature pause/resume state machine was validated with temporary 27C/25C thresholds; release thresholds remain 60C stop / 55C resume.
- Successful `[TEMP]` logs are throttled to once per 1000 ms to reduce serial spam during cooldown. Safety/fault events remain immediate.
- `GET_TEMP` forces one temperature log and continues to return the live measured temperature.
