# PRD-118 - Serial Station Profile Service

## 背景

Serial Station 已经具备主入口、快速启动、UART 配置、协议选择、命令发送、日志、导出和回放预览能力。但这些能力仍偏“临时会话”：用户每次进入工站后需要重新确认端口、波特率、协议和常用命令，缺少企业级上位机常见的“设备/工站配置档案”能力。

本轮先在 `services/` 层建立稳定配置档案服务，为后续 UI 档案列表、一键应用和团队共享 JSON 文件打基础。

## 目标

1. 新增 `SerialProfileService`，保存和恢复 Serial Station 工站档案。
2. 档案包含名称、描述、标签、UART 配置、默认协议、默认发送模式、常用命令列表。
3. 支持 JSON 对象序列化、JSON 文本解析、文件保存和文件加载。
4. 校验缺失名称、非法 UART 配置、非法协议名和非法命令项，返回明确错误。
5. 复用既有 `SerialPortConfig`，不复制第二套 UART 参数结构。

## 非目标

1. 不新增 UI 面板、按钮或弹窗。
2. 不自动连接真实串口。
3. 不修改协议实现或协议注册表。
4. 不修改 `SerialStationController` 或 `SerialStationWindow` 工作流。
5. 不提升设备验证状态。

## 架构边界

- 本轮属于 Serial Station `services/` 层。
- `SerialProfileService` 只使用 Qt Core、Qt SerialPort 类型和 `SerialPortConfig` 值对象。
- 服务层不 include `ui/`、`core/SerialManager`、具体协议目录或 QWidget。
- 文件写入使用 Qt 文件 API，保持在 service 层内。
- 测试放在 `tests/serial_station/`，不依赖真实 COM 口。

## 验收标准

1. 新增 QTest 覆盖档案 JSON round-trip。
2. 新增 QTest 覆盖常用命令顺序、标签、协议和发送模式保持。
3. 新增 QTest 覆盖缺失名称、非法 UART、坏 JSON、文件缺失等错误路径。
4. 新增 `.h/.cpp` 已加入 `cmake/EmbedDebugSources.cmake` 和 `tests/CMakeLists.txt`。
5. `cmake --build build --target test_serial_profile_service --parallel 4` 通过。
6. `.\build\tests\test_serial_profile_service.exe` 通过。
7. `cmake --build build --target EmbedDebug --parallel 4` 通过。
8. `.\EmbedDebug.bat` 启动探针通过。
9. 根目录只存在 `build/`。

## 三轴状态

- 工程状态：`E3 -> E4`，以 service QTest、CMake 注册和主目标构建为证据。
- 用户状态：不提升。本轮是服务层能力，后续 UI 接入后再提升体验完整度。
- 设备状态：不提升，真实硬件未验证。

## 收口记录

- 红灯：`cmake --build build --target test_serial_profile_service --parallel 4` 先因缺少 `SerialProfileService.cpp` 失败。
- `test_serial_profile_service`: 12 passed, 0 failed。
- `cmake --build build --target EmbedDebug --parallel 4`: passed。
- service include 边界检查：未发现 `ui/`、`SerialStationWindow`、`SerialStationController`、`core/SerialManager` 或具体协议目录 include。
- `.\EmbedDebug.bat`: exit=0，启动进程后探针关闭。
- 根目录构建目录：仅 `build/`。
