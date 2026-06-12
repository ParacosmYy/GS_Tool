# PRD-083 - Serial Station Workbench Shell

## 背景

当前 `src/apps/serial_station/` 已有 UART 配置面板和基础 controller/core/protocol 骨架，但顶层窗口仍只展示单个配置表单。用户打开新串口工站时，看不到命令发送、日志查看、会话摘要等上位机核心工作区，体感仍像“参数页”而不是“串口调试台”。

本 PRD 目标是补齐 Serial Station 第一版可见工作台壳层，让后续协议、日志、回放和命令能力可以在明确 UI 边界内继续接入。

## 目标

1. 在 `SerialStationWindow` 中形成稳定的工作台布局：
   - 左侧/侧栏：复用现有 `SerialPortPanel` 做 UART 参数配置。
   - 中央：新增命令发送面板，承载发送输入、格式模式、快捷命令入口。
   - 中央/下方：新增日志面板，承载收发记录、过滤、清空和导出入口。
   - 底部：新增状态栏，展示连接状态、端口、波特率、发送/接收计数和错误计数。
2. 新 UI 面板只发出用户意图 signal，不直接拼接协议帧、不直接调用 `SerialManager`、不写文件。
3. 所有新增控件设置稳定 `objectName`，用户可见文字使用 `tr()`。
4. 三套主题至少能识别新增 objectName，保持工作台层级、按钮状态和日志区域可读。
5. 新增源码必须加入 `CMakeLists.txt`，并配套 QTest 覆盖基础 UI 行为。

## 非目标

1. 不实现真实串口发送队列。
2. 不新增 Modbus/custom 协议实现。
3. 不接真实日志导出文件写入。
4. 不迁移旧 `src/serial/` 模块。
5. 不修改 `EmbedDebug.bat`、构建目录或可执行文件输出路径。

## 分层边界

本轮主改动层级为 `ui` 和 App 装配层：

- `ui/SerialCommandPanel.*`：只收集命令文本、发送模式和用户点击意图。
- `ui/SerialLogPanel.*`：只展示/过滤/清空 UI 内记录，不直接写文件。
- `ui/SerialStatusBar.*`：只展示会话摘要。
- `SerialStationWindow.*`：只装配面板并连接 UI 层信号到临时 UI 反馈。

禁止事项：

- UI 不 include `core/SerialManager.h` 或具体协议头文件。
- UI 不拼 `QByteArray` 协议帧。
- UI 不直接写文件。
- `core/` 不 include 新 UI 文件。

## 验收标准

1. `SerialStationWindow` 不再只有单个 UART 配置表单，而是包含配置、命令、日志、状态栏四类区域。
2. 新增面板的核心子控件可通过 QTest `findChild` 找到。
3. 点击发送按钮会发出 UI 意图 signal，并在日志面板产生本地 UI 预览记录。
4. 清空日志按钮能清空日志显示。
5. `cmake --build build --target EmbedDebug --parallel 4` 通过。
6. Serial Station 相关 QTest 通过。
7. `tools/doctor.ps1` 通过，允许保留既有 `go.exe` warning。
8. `EmbedDebug.bat` 启动验证通过。

## 失败条件

1. 新 UI 层直接依赖 `SerialManager`、具体协议或文件服务。
2. 新增 `.h/.cpp` 未加入 `CMakeLists.txt`。
3. 构建失败或 `EmbedDebug.bat` 启动失败。
4. 单个新增 `.cpp` 超过 500 行或 `.h` 超过 200 行。
5. 只做样式、不形成新的工作台可用结构。
