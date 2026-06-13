# PRD-119 - Serial Station Profile UI Flow

## 背景

PRD-118 已经在 `services/` 层落地 `SerialProfileService`，可以保存和恢复 UART、协议、发送模式和常用命令档案。但用户仍无法从 Serial Station 工作台直接使用该能力，服务层能力还没有转化成“工站配置齐全”的可见路径。

本轮目标是在不破坏分层边界的前提下，完成配置档案的最小 UI 闭环：窗口提供保存/加载入口，Controller 调用 profile service，UI 面板负责应用值对象。

## 目标

1. Serial Station 工作台顶部显示“保存档案”和“加载档案”入口。
2. 支持保存当前 UART 配置、激活协议、当前发送模式和最近命令到档案文件。
3. 支持加载档案文件后回填 UART 配置、协议选择、默认发送模式和命令历史。
4. 成功/失败结果写入工作台日志，不弹出不统一的业务错误。
5. 继续复用 `SerialProfileService`，不在 UI 里解析 JSON。

## 非目标

1. 不实现档案列表管理、删除、重命名或搜索。
2. 不自动连接串口。
3. 不新增协议或改变协议解析。
4. 不引入真实硬件验证。

## 架构边界

- UI 面板只提供 current/apply 方法和值对象 signal，不写文件、不解析 JSON。
- `SerialStationWindow` 只做装配、对话框路径选择和控件间协调。
- `SerialStationController` 持有并调用 `SerialProfileService`。
- `services/SerialProfileService` 仍是 JSON 和文件读写唯一入口。
- 本轮跨 `ui/controller/services` 是为了完成用户路径，调用链写入 Specs 并由窗口统一装配。

## 验收标准

1. QTest 覆盖档案保存/加载按钮存在。
2. QTest 覆盖加载档案后 UART、协议、发送模式和命令历史回填。
3. QTest 覆盖保存当前工作台状态生成可由 `SerialProfileService` 读取的档案。
4. QTest 覆盖加载坏档案不会污染当前 UI 状态。
5. `test_serial_station_workbench` 构建和运行通过。
6. `EmbedDebug` 主目标构建通过。
7. `EmbedDebug.bat --station serial` 启动探针通过。

## 三轴状态

- 工程状态：`E4 -> E5`，以服务复用、UI/Controller 边界和工作台 QTest 为证据。
- 用户状态：`U3 -> U4`，用户可在工作台完成配置档案保存/加载主路径。
- 设备状态：不提升，真实硬件未验证。

## 收口记录

- 红灯测试：`test_serial_station_workbench` 首次构建失败，原因是 `SerialStationWindow` 尚未提供档案保存/加载测试入口。
- QTest：`test_serial_command_panel` 25/25、`test_serial_port_panel` 27/27、`test_serial_station_workbench` 22/22 通过。
- 主目标：`cmake --build build --target EmbedDebug --parallel 4` 通过。
- 边界检查：UI 未 include `core/SerialManager`、具体协议目录或 `SerialProfileService`；profile service 未 include UI/QWidget/具体协议目录。
- 启动入口：`EmbedDebug.bat --station serial` 启动探针通过，退出码 0。
- 构建目录：根目录仅存在 `build/`。
