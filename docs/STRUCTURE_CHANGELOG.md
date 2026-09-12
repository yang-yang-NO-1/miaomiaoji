# V6 Code Organization Changelog

- 将 `main.h` 拆分为 HardwarePins / PrinterConfig / AppState / Sensors / Printer / PaperangProtocol。
- 将散落在 `Paperang.cpp` 的共享全局变量迁移到 `AppState.cpp`。
- 将 `OtherFunction.cpp` 重命名为 `Sensors.cpp`。
- `main.cpp` 只保留硬件初始化、PSRAM缓存申请和协议入口。
- 删除已失效的大段注释掉的打印缓存任务代码，降低阅读噪声。
- 移除 `testPage()` 中未使用变量警告。
- `CMD_42_DATA` 改为 const 数组，消除字符串常量转 `char*` 警告，数据内容不变。
- 对旧工程 `9999999999 -> uint32_t` 的截断行为改成显式转换，保持旧值语义同时避免隐式溢出警告。
- 文档和硬件资料统一移入 `docs/`。
- 未修改温度、缺纸、P2缩放、标签补走等已实机验证行为。
- 删除 PlatformIO 自动生成且绑定原电脑绝对路径的 `.vscode/c_cpp_properties.json` 与 `.vscode/launch.json`；PlatformIO 会在新目录中自动重建。
