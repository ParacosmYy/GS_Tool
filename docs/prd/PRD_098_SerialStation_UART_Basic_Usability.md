# PRD_098 - Serial Station UART Basic Usability

## 1. 背景

用户反馈 Serial Station “最基础的 UART 都没有”。现有 `SerialPortPanel` 已有端口、波特率、数据位、校验、停止位、流控、DTR/RTS 控件，但基础体验不足：

- 端口列表只显示 `COMx`，缺少描述、制造商、VID/PID 等识别信息。
- 没有稳定展示“当前 UART 配置摘要”，用户点击连接前难以确认 115200 8N1 等基础参数。
- 无端口时虽然允许手动输入，但缺少明确的手动模式反馈。
- 刷新动作只在面板内部消费，上层无法展示刷新结果。
- 测试覆盖偏少，不能证明基础 UART 面板作为上位机入口足够可用。

本阶段优先修复最基础 UART 使用体验，不改协议、不改发送队列、不碰真实设备依赖。

## 2. 目标

1. `SerialPortPanel` 端口下拉项显示端口名和可读设备描述。
2. 当前配置仍以真实 `portName` 作为数据值，连接时不把描述文本当端口名。
3. 无端口时保持可手动输入 COM 名，并明确提示“手动输入模式”。
4. 增加 UART 配置摘要标签，展示端口、波特率、帧格式、流控、DTR/RTS。
5. 用户修改端口、波特率、数据位、校验、停止位、流控、控制线时摘要即时更新。
6. 刷新按钮发出 `refreshRequested()`，测试覆盖刷新信号和基础控件状态。
7. README 同步声明 Serial Station 具备基础 UART 配置与手动端口输入能力。

## 3. 非目标

- 不新增后台串口扫描线程。
- 不依赖真实 COM 口测试。
- 不实现自动重连。
- 不实现串口高级监控线图。
- 不修改 `core/SerialManager` 打开/关闭行为，除非发现配置值错误。
- 不新增协议、worker 或真实回放发送。

## 4. 分层边界

本次改动主要属于 `ui` 层，少量涉及 README 和测试。

- UI 只收集 UART 参数并发出 `connectRequested(config)`。
- UI 不直接调用 `SerialManager`。
- Controller、core、protocols 本阶段不新增业务。
- `SerialPortConfig` 继续作为 UART 参数值对象，不承担端口枚举职责。

## 5. 验收标准

- 默认配置为 `115200 8N1 无流控 DTR=off RTS=off`。
- 端口 Combo 的 item data 保存真实端口名。
- 显示文本可包含描述，但 `currentConfig().portName` 必须是端口名或手动输入名。
- 无端口时端口 Combo 可编辑，状态提示明确可手动输入。
- 摘要标签 objectName 为 `serialUartSummaryLabel`。
- 修改波特率、帧格式、流控、DTR/RTS 后摘要即时变化。
- 点击刷新按钮发出 `refreshRequested()`。
- 打开状态禁用配置控件，关闭状态恢复配置控件。
- QTest 不依赖真实 COM 口。
- `EmbedDebug.bat` 启动链路仍通过。

## 6. 失败条件

- UI 把 `COM3 - USB Serial` 这种显示文本传给 `QSerialPort::setPortName()`。
- 无真实串口时无法手动输入端口。
- 用户无法从 UI 看出当前 UART 参数。
- 测试依赖本机必须存在某个 COM 口。
- 为了修 UI 越界修改 core/protocol/worker。
