# PRD-084 - Serial Station Controller Send Loop

## 背景

PRD-083 已经把 Serial Station 从单个 UART 配置页推进为四区工作台，但命令区的发送动作仍停留在 UI 本地预览。用户点击发送后没有经过 `SerialStationController`、`SerialManager` 和协议层，导致新串口工站看起来像界面壳，而不是可以逐步接真实串口的上位机。

本 PRD 目标是补齐第一版 controller 发送闭环：UI 只发出发送意图，controller 负责校验、协议构帧、串口发送和日志/状态信号，core/protocols 继续保持在既有边界内。

## 目标

1. `SerialCommandPanel` 的发送信号接入 `SerialStationController`，不再由 `SerialStationWindow` 做本地发送预览。
2. `SerialStationController` 新增发送入口，完成：
   - 命令文本和发送模式校验。
   - 选择当前协议或默认协议构建发送帧。
   - 调用 `SerialManager` 发送 `QByteArray`。
   - 对 UI 发出发送成功、发送失败、日志追加和状态更新信号。
3. 未打开串口、空命令、未知模式、协议不可用、发送失败等路径必须有明确错误信号。
4. UI 通过 signal/slot 接收 controller 结果，更新日志区和状态栏。
5. 增加 QTest 覆盖 controller 发送校验路径和窗口连接行为。

## 非目标

1. 不新增 Modbus、二进制帧或自定义协议实现。
2. 不新增发送队列、定时发送、重发、超时等待或 worker 线程。
3. 不做日志文件导出、回放或会话持久化。
4. 不迁移旧 `src/serial/` 模块。
5. 不修改启动脚本、构建目录和可执行文件输出路径。

## 分层边界

本轮主改动层级为 `ui`、`controller` 和既有 `core/protocols` 调用链：

- UI：只发出 `sendRequested(command, mode)`，只展示 controller 回传的日志和状态。
- Controller：负责输入校验、协议选择、发送协调和错误归一。
- Core：继续只处理 `QByteArray` 发送和串口状态。
- Protocols：继续只处理命令构建、帧定义和解析。

禁止事项：

- UI include `core/SerialManager.h` 或具体协议头。
- UI 直接拼接 `QByteArray` 发送帧。
- Controller include 具体协议目录，必须经 registry 或接口。
- Core include UI 或具体协议目录。
- Protocols include UI、`SerialStationWindow` 或 `SerialStationController`。

## 验收标准

1. 点击命令区发送按钮后，调用链为 UI signal -> `SerialStationController` -> protocol/core，而不是窗口本地预览。
2. 串口未打开时发送失败，日志区出现可见错误，状态栏错误计数增加。
3. 空命令不会发送，日志区出现可见错误。
4. 合法命令在串口打开场景下由 controller 调用 `SerialManager::send`。
5. 新增或更新 QTest 能覆盖 controller 发送失败路径和窗口信号连接。
6. `cmake --build build --target EmbedDebug --parallel 4` 通过。
7. Serial Station 相关 QTest 通过。
8. `tools/doctor.ps1` 通过，允许保留既有 `go.exe` warning。
9. `EmbedDebug.bat` 启动验证通过。

## 失败条件

1. UI 绕过 controller 直接调用 core/protocol。
2. Controller 直接依赖具体协议目录。
3. 为发送闭环新增第二套 Hex、CRC、RingBuffer 或 Settings 工具。
4. 新增 `.h/.cpp` 未加入 `CMakeLists.txt`。
5. 构建失败、测试失败或启动验证失败。
