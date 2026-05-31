# PRD-031: 串口功能无死角完善

## 背景

commit #30 已完成 PanelManager 行数压缩和注释规范化，当前评分 32 分。

用户核心诉求: **"先要百分之百保证我的串口可以无死角的使用跟配置"**。

经全面审查 `SerialConfigPanel`、`SerialConnection`、`ConnectionController`、`SendController`、`TimedSender`、`SendHistory`、`DataStatistics`、`QuickCommandBar`、`SerialDriverDetector`、`TerminalWidget`、`TerminalModel`、`TerminalSearchBar` 全部串口相关源码后，发现以下问题需要系统性解决:

**已实现的功能（代码存在且逻辑完整）**:
- SerialConfigPanel: 端口选择/刷新、波特率(可编辑)/数据位/校验/停止位/流控/DTR/RTS 八项配置项 UI 完整
- SerialConnection: QSerialPort 完整封装，支持 configure(QVariantMap) 统一参数设置
- ConnectionController: 串口创建/断开/信号连接/DTR/RTS 运行时控制
- SendController: 文本/HEX 双模式发送、换行符自动追加(\r\n/\n/\r)、发送历史自动补全
- TimedSender: 定时循环发送，支持多组数据队列
- SendHistory: 去重记录、关键词搜索、最大条数限制
- DataStatistics: RX/TX 累计字节数、实时速率、连接持续时间
- QuickCommandBar: 可配置快捷指令按钮、HEX/文本双模式
- SerialDriverDetector: CH340/CP2102/FT232/PL2303 等驱动检测
- TerminalWidget: 自绘制引擎、Text/Hex/Mixed/Decimal 四种显示模式、时间戳、方向前缀、搜索高亮
- TerminalSearchBar: 文本/正则/HEX 三种搜索模式、匹配导航

**存在的缺陷和缺失（本 PRD 需要解决）**:

1. **波特率输入无校验** -- `m_baudCombo->setEditable(true)` 允许自由输入但无任何校验，用户输入 "abc" 或 "0" 或负数不会报错，`currentBaudRate()` 调用 `toInt()` 静默返回 0，导致串口以 0 波特率打开，行为不可预测。

2. **端口被占用时错误信息不友好** -- `SerialConnection::open()` 失败时直接转发 `QSerialPort::errorString()`，用户看到的是 "The resource is already in use" 这类 Qt 底层英文错误，缺少中文上下文（哪个端口、为什么失败、如何解决）。

3. **断开后重连无自动重连机制** -- USB 线物理拔出或设备复位时，`onError()` 将状态设为 Error 后无进一步动作。用户必须手动点"断开"再点"连接"，体验割裂。

4. **连接状态未回写到 SerialConfigPanel** -- `ConnectionController::connectSerial()` 发出 `connectionStateChanged` 信号，但 `SerialConfigPanel::setConnected()` 只由 MainWindow 显式调用，如果 MainWindow 遗漏调用（或异常路径跳过），面板状态和实际连接状态不一致。

5. **定时发送无 UI 控件** -- `TimedSender` 类功能完整（间隔/队列/启停），但发送栏没有暴露任何 UI 入口让用户设置间隔时间、配置队列、启动/停止定时发送。定时发送功能实际不可达。

6. **快捷指令无持久化** -- `QuickCommandBar::addCommand()` 添加的指令仅在内存中，应用重启后全部丢失，用户每次都要重新配置。

7. **快捷指令编辑体验缺失** -- 只有"添加默认指令"和"编辑请求"信号，没有实际的编辑对话框实现，用户无法修改已有指令的名称、数据、HEX 标记。

8. **发送历史不跨会话持久化** -- `SendHistory` 数据仅在内存中，最大 50 条，应用重启即清空。

9. **HEX 发送输入无实时校验反馈** -- 用户输入非法 HEX 字符（如 "GG" 或 "1 2 3"）时，只有点发送后才提示失败，没有输入过程中的实时格式校验和提示。

10. **接收数据统计与 DataStatistics 面板未联动** -- `DataStatistics::update(rxBytes, txBytes)` 需要外部传入累计值，但从代码中未见 MainWindow 或 ConnectionController 将 `TerminalModel::rxBytes()/txBytes()` 传递给 `DataStatistics::update()` 的调用链。DataStatistics 面板可能一直显示 "0 B"。

11. **串口配置不跨会话保存** -- `SerialConfigPanel::restoreConfig()` 方法存在且逻辑完整，但从代码中未见 MainWindow 在启动时调用 `restoreConfig()` 将上次使用的端口/波特率等参数恢复到面板。

12. **驱动检测信息缺少可操作指引** -- `SerialDriverDetector::driverStatusSummary()` 检测到无驱动时给出英文提示，但未提供驱动下载链接或一键跳转到驱动安装向导。

**审查基准**: commit #30, score 32。

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 串口配置面板完善 -- 波特率输入校验、配置跨会话持久化、连接状态双向同步、端口被占用友好提示 | P0 | serial/SerialConfigPanel.h/cpp, core/ConnectionController.h/cpp |
| R2 | 串口连接稳定性 -- 自动重连机制、错误恢复、异常断开检测与通知、连接超时控制 | P0 | connection/SerialConnection.h/cpp, core/ConnectionController.h/cpp |
| R3 | 发送功能完善 -- 定时发送 UI 入口、HEX 输入实时校验、发送历史持久化、快捷指令持久化与编辑对话框 | P0 | core/SendController.h/cpp, serial/TimedSender.h/cpp, serial/QuickCommandBar.h/cpp, serial/SendHistory.h/cpp |
| R4 | 接收功能完善 -- DataStatistics 联动、显示模式切换 UI 入口确认、时间戳/方向前缀开关确认 | P1 | serial/DataStatistics.h/cpp, terminal/TerminalWidget.h/cpp, core/MainWindow.cpp |
| R5 | 串口驱动检测增强 -- 中文提示、驱动下载链接、无端口时诊断指引 | P1 | serial/SerialDriverDetector.h/cpp |

---

## 需求详细说明

---

### R1: 串口配置面板完善 (P0)

#### R1.1 波特率输入校验

**问题**: `m_baudCombo->setEditable(true)` 允许自由输入，`currentBaudRate()` 调用 `toInt()` 对非法输入静默返回 0。

**方案**:

1. 在 `SerialConfigPanel` 中为 `m_baudCombo` 添加 `QIntValidator`，限制输入范围 300 ~ 12000000（覆盖常用和特殊波特率）。
2. 在 `currentBaudRate()` 方法中增加防御性检查: 如果 `toInt()` 返回 0 或负数，回退到 115200 默认值并发出警告日志。
3. 连接 `m_baudCombo::currentTextChanged` 信号，实时验证输入，非法值时通过 `setProperty("hasError", true)` 触发 QSS 错误样式（红色边框），合法值时清除错误状态。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R1.1-01 | 在波特率框输入 "abc" | 输入被 QIntValidator 拦截，无法输入非数字字符 |
| TC-R1.1-02 | 在波特率框输入 "9600" | 正常接受，currentBaudRate() 返回 9600 |
| TC-R1.1-03 | 在波特率框输入 "0" | 输入框显示红色边框，连接时使用回退值 115200 |
| TC-R1.1-04 | 在波特率框输入 "-100" | 输入被 QIntValidator 拦截，无法输入负号 |
| TC-R1.1-05 | 在波特率框输入 "12000001" | 超过上限，输入框显示红色边框 |
| TC-R1.1-06 | 在波特率框输入 "" (清空) | 输入框显示红色边框，连接时使用回退值 115200 |
| TC-R1.1-07 | 从下拉列表选择 "921600" | 正常切换，无错误样式 |
| TC-R1.1-08 | 输入自定义波特率 "256000" | 正常接受，能成功连接 |

**验收标准**:
- 波特率输入框只接受正整数输入
- 非法输入有视觉反馈（红色边框）
- `currentBaudRate()` 永远不会返回 0 或负数
- 自定义波特率（如 256000、500000）可正常使用

---

#### R1.2 串口配置跨会话持久化

**问题**: `restoreConfig()` 方法存在但未被调用，用户每次启动应用都需重新选择端口和参数。

**方案**:

1. 在 `ConnectionController::connectSerial()` 成功连接后，将当前参数通过 `SettingsManager` 保存到 `embeddebug_settings.json`，key 为 `serial/lastConfig`。
2. 在 MainWindow 初始化时（`SerialConfigPanel` 构造完成后），从 `SettingsManager` 读取 `serial/lastConfig`，调用 `restoreConfig()` 恢复面板状态。
3. 保存内容: `portName`(string), `baudRate`(int), `dataBits`(int: 0-3), `parity`(int: 0-4), `stopBits`(int: 0-2), `flowControl`(int: 0-2), `dtr`(bool), `rts`(bool)。
4. 端口名恢复时，如果上次使用的端口本次不存在（USB 设备未插入），静默跳过端口恢复，其他参数正常恢复。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R1.2-01 | 配置 COM3/115200/8N1/无流控/DTR+RTS → 连接成功 → 关闭应用 → 重新打开 | 面板自动恢复为 COM3/115200/8N1/无流控/DTR+RTS |
| TC-R1.2-02 | 配置 COM5 → 连接 → 关闭 → 拔掉 USB 设备 → 重新打开 | 面板波特率/数据位等参数恢复，端口下拉框不选中任何项（COM5 不存在） |
| TC-R1.2-03 | 首次安装应用，无 settings 文件 | 面板显示默认值: 115200/8/无校验/1停止位/无流控/DTR+RTS |
| TC-R1.2-04 | 配置 921600/7/偶校验/2停止位 → 连接 → 断开 → 关闭 → 重新打开 | 面板恢复为 921600/7/偶校验/2停止位 |
| TC-R1.2-05 | 修改配置后不连接直接关闭 | 上次成功连接的配置被保留（未连接不覆盖保存值） |

**验收标准**:
- 应用重启后，上次成功连接的串口参数自动恢复到面板
- 端口不存在时优雅降级，不影响其他参数恢复
- 配置文件损坏时，应用正常启动，使用默认值

---

#### R1.3 连接状态双向同步

**问题**: `SerialConfigPanel::setConnected()` 依赖 MainWindow 显式调用，存在状态不一致风险。

**方案**:

1. 让 `ConnectionController` 在连接成功、断开、错误时直接发出信号 `connectionStateChanged(state, name)`。
2. MainWindow 接收该信号后调用 `SerialConfigPanel::setConnected(state == Connected)`。
3. 在 `SerialConfigPanel` 中增加 `ConnectionState` 的细粒度感知: 连接中时按钮显示 "连接中..." 并禁用，连接成功时显示 "断开" 并启用，错误时显示 "重试连接" 并高亮。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R1.3-01 | 点击"连接"按钮 | 按钮文字变为"连接中..."，所有参数控件禁用 |
| TC-R1.3-02 | 连接成功 | 按钮文字变为"断开"，DTR/RTS 保持可用 |
| TC-R1.3-03 | 连接失败（端口不存在） | 按钮恢复为"连接"，参数控件恢复可用，显示错误提示 |
| TC-R1.3-04 | 连接成功后拔掉 USB | 按钮变为"重试连接"样式（error 色边框），状态指示更新 |
| TC-R1.3-05 | 点击"断开" | 按钮恢复为"连接"，所有参数控件恢复可用 |

**验收标准**:
- 任何连接状态变化，SerialConfigPanel 都能正确反映（按钮文字 + 控件可用性）
- 无 MainWindow 遗漏调用的可能性，状态同步由信号驱动

---

#### R1.4 端口占用/错误友好提示

**问题**: `SerialConnection::open()` 失败时直接转发 Qt 英文错误字符串。

**方案**:

1. 在 `SerialConnection::open()` 中，对常见错误码映射为中文友好提示:

| QSerialPort 错误 | 中文提示 | 建议操作 |
|-----------------|---------|---------|
| DeviceNotFoundError | "端口 {name} 不存在" | "请检查设备是否已连接，或点击刷新按钮重新扫描" |
| PermissionError | "端口 {name} 被其他程序占用" | "请关闭其他串口工具（如 SSCOM、Putty），或检查是否有其他 EmbedDebug 实例正在运行" |
| OpenError | "无法打开端口 {name}" | "请检查端口是否可用，或尝试重新插拔设备" |
| ResourceError | "端口 {name} 发生资源错误" | "设备可能已断开，请检查 USB 连接" |
| UnsupportedOperationError | "不支持的操作" | "当前配置不被系统支持" |
| UnknownError | "未知错误: {原始错误}" | "请重启应用或重新插拔设备" |

2. 在 `ConnectionController::connectSerial()` 中，将 `connectionFailed` 信号携带的 title/message 用中文友好提示填充，替代通用 "Connection Failed"。

3. MainWindow 接收 `connectionFailed` 后，使用 Toast 通知（而非阻塞对话框）展示错误，3 秒后自动消失。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R1.4-01 | 连接一个已被 SSCOM 打开的 COM 口 | Toast 提示 "端口 COM3 被其他程序占用"，建议关闭 SSCOM |
| TC-R1.4-02 | 连接一个不存在的 COM 口 | Toast 提示 "端口 COM99 不存在"，建议检查设备连接 |
| TC-R1.4-03 | 连接过程中拔掉 USB | Toast 提示 "端口 COM3 发生资源错误"，建议检查 USB |
| TC-R1.4-04 | 使用不支持的波特率/参数组合 | Toast 提示 "不支持的操作"，建议调整参数 |

**验收标准**:
- 所有串口连接错误都以中文展示
- 每个错误消息都附带具体的解决建议
- 使用 Toast 通知，不使用阻塞式 QMessageBox

---

### R2: 串口连接稳定性 (P0)

#### R2.1 自动重连机制

**问题**: USB 拔出或设备复位后，状态变为 Error，用户必须手动操作。

**方案**:

1. 在 `SerialConnection::onError()` 中，对 `ResourceError`（设备意外断开）和 `TimeoutError` 启动自动重连逻辑。
2. 在 `ConnectionController` 中增加自动重连状态机:

```
[已连接] --设备断开--> [等待重连] --间隔1s--> [尝试重连] --成功--> [已连接]
                                          |
                                          +--失败--> [等待重连] (累计重试N次)
                                                      |
                                                      +--超过最大重试次数--> [放弃重连，通知用户]
```

3. 重连参数:
   - 首次重连延迟: 1 秒
   - 最大重试次数: 10 次
   - 重试间隔递增: 1s, 1s, 2s, 2s, 4s, 4s, 8s, 8s, 16s, 16s（指数退避，上限 16s）
   - 使用上一次成功的连接参数自动重连

4. 重连状态通过 `connectionStateChanged(Connecting, name)` 信号通知 UI，面板按钮显示 "重连中(3/10)..."。

5. 用户可随时点击 "取消重连" 按钮终止自动重连。

6. 用户手动点击 "断开" 时不触发自动重连（区分主动断开和异常断开）。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R2.1-01 | 连接 COM3 → 拔掉 USB → 1s 内开始自动重连 | 按钮显示 "重连中(1/10)..."，面板参数禁用 |
| TC-R2.1-02 | 自动重连中，重新插入 USB | 自动检测到端口可用，重连成功，按钮变为 "断开" |
| TC-R2.1-03 | 自动重连 10 次仍失败 | 停止重连，按钮变为 "连接"，Toast 提示 "自动重连失败，请检查设备" |
| TC-R2.1-04 | 自动重连过程中点击 "取消重连" | 立即停止重连，按钮恢复为 "连接" |
| TC-R2.1-05 | 用户手动点击 "断开" | 不触发自动重连，按钮直接变为 "连接" |
| TC-R2.1-06 | 重连过程中应用不卡顿，UI 响应流畅 | 重连在后台线程或定时器中执行，主线程无阻塞 |

**验收标准**:
- USB 拔出后自动尝试重连，无需用户干预
- 重连成功后，DTR/RTS 状态恢复到断开前的设置
- 重连过程中 UI 有明确的状态指示
- 用户可随时中断重连
- 重连失败有明确提示

---

#### R2.2 错误恢复与异常处理

**问题**: `SerialConnection::onError()` 将状态设为 Error 后，QSerialPort 对象可能处于不一致状态。

**方案**:

1. 在 `onError()` 中，对非 `NoError` 的错误增加以下处理:
   - 调用 `m_serial.close()` 确保端口完全释放
   - 将 `m_state` 设为 `Disconnected`（而非 Error），让上层可以尝试重连
   - 在关闭前缓存错误信息，关闭后发出 `errorOccurred` 和 `stateChanged`

2. 在 `ConnectionController` 中，`connectSerial()` 增加连接超时保护:
   - 如果 `open()` 调用后 5 秒内未收到 `stateChanged(Connected)` 信号，视为超时
   - 使用 `QTimer::singleShot(5000, ...)` 实现超时检测
   - 超时后关闭端口，发出 `connectionFailed` 信号

3. 在 `SerialConnection::write()` 中增加错误检查:
   - 检查 `m_serial.error()` 状态，如果有错误则不写入
   - 写入后检查返回值，`-1` 时发出错误信号
   - 增加写入超时机制: 如果 `m_serial.bytesToWrite()` 持续大于 0 超过 3 秒，视为写入超时

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R2.2-01 | 连接过程中设备无响应（如波特率严重不匹配） | 5 秒后超时，提示连接超时 |
| TC-R2.2-02 | 连接中发送数据时设备突然断开 | write() 返回 -1，错误被捕获，触发自动重连 |
| TC-R2.2-03 | 连接中设备频繁断开又恢复（如接触不良） | 自动重连能正确处理多次断开/恢复循环 |
| TC-R2.2-04 | 连接中关闭应用 | 析构函数中 close() 被调用，无崩溃无内存泄漏 |
| TC-R2.2-05 | 连接中系统休眠再恢复 | 恢复后自动尝试重连 |

**验收标准**:
- 所有串口异常都有对应的处理路径，无未捕获的错误
- 异常处理后，SerialConnection 对象状态一致，可安全重连
- 无内存泄漏、无崩溃、无死锁

---

### R3: 发送功能完善 (P0)

#### R3.1 定时发送 UI 入口

**问题**: `TimedSender` 类功能完整，但无 UI 控件暴露给用户。

**方案**:

1. 在 `SendController::createSendBar()` 中，在发送按钮右侧增加定时发送控件组:
   - 定时发送开关按钮（Toggle Button），图标式，启用时高亮 accent 色
   - 间隔时间输入框（QSpinBox），单位: 毫秒，范围: 50 ~ 60000，默认: 1000
   - 可选: 循环次数设置（0 = 无限循环）

2. 控件布局: `[模式切换] [换行符] [输入框] [发送按钮] [定时开关⏱] [间隔: [1000]ms]`

3. 定时发送开关状态:
   - 未连接时: 按钮禁用（灰色）
   - 已连接且关闭定时: 按钮正常态（点击开启）
   - 已连接且开启定时: 按钮高亮态（accent 色，点击关闭）

4. 定时发送数据源为当前发送输入框的内容。用户修改输入框内容后，定时发送自动使用新内容。

5. 定时发送过程中，输入框仍可手动发送（两者不冲突）。

6. 连接断开时，自动停止定时发送。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R3.1-01 | 输入 "AT" → 开启定时发送（间隔 500ms） | 每 500ms 发送一次 "AT"，终端显示 TX 数据 |
| TC-R3.1-02 | 定时发送中修改间隔为 100ms | 发送频率立即变为 100ms |
| TC-R3.1-03 | 定时发送中修改输入框内容为 "AT+RST" | 下一次定时触发时发送 "AT+RST" |
| TC-R3.1-04 | 定时发送中断开连接 | 定时发送自动停止，定时按钮恢复为关闭态 |
| TC-R3.1-05 | 未连接时点击定时发送按钮 | 按钮无响应（禁用态） |
| TC-R3.1-06 | HEX 模式下开启定时发送 | 按 HEX 格式解析输入框内容后定时发送 |
| TC-R3.1-07 | 设置间隔为 50ms → 快速发送 1000 次 | 无内存泄漏，无缓冲区溢出，发送间隔准确 |

**验收标准**:
- 用户可通过 UI 完整控制定时发送的开启/关闭/间隔
- 定时发送状态在 UI 上有明确视觉指示
- 断开连接后定时发送自动停止
- 定时发送过程中不阻塞 UI

---

#### R3.2 HEX 输入实时校验

**问题**: 非法 HEX 输入只有点发送后才提示。

**方案**:

1. 在 `SendController` 中，当 `m_sendModeCombo` 切换到 HEX 模式时，连接 `m_sendInput::textChanged` 信号到校验槽函数。
2. 校验规则:
   - 允许的字符: 0-9, a-f, A-F, 空格
   - HEX 字符个数必须为偶数（每两个字符代表一个字节）
   - 空格分隔可选（"AA BB" 和 "AABB" 都合法）
3. 校验结果实时反映在输入框:
   - 合法: 正常样式
   - 非法字符: 输入框边框变红，tooltip 显示 "包含非法字符: 只允许 0-9, a-f, A-F 和空格"
   - 奇数个字符: 输入框边框变黄，tooltip 显示 "HEX 字符数必须为偶数"
4. 文本模式下不触发校验。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R3.2-01 | HEX 模式输入 "AA BB CC" | 输入框正常样式 |
| TC-R3.2-02 | HEX 模式输入 "AABBCC" | 输入框正常样式 |
| TC-R3.2-03 | HEX 模式输入 "GG" | 输入框红色边框，提示非法字符 |
| TC-R3.2-04 | HEX 模式输入 "ABC" | 输入框黄色边框，提示奇数个字符 |
| TC-R3.2-05 | HEX 模式输入 "A B C" | 输入框黄色边框（去空格后 "ABC" 为 3 个字符） |
| TC-R3.2-06 | 切换到文本模式输入任意内容 | 无校验反馈，正常样式 |
| TC-R3.2-07 | HEX 模式输入 " " (纯空格) | 输入框正常样式（空内容不校验） |

**验收标准**:
- HEX 输入有实时校验反馈，无需点击发送
- 非法输入和不完整输入有不同的视觉提示（红色 vs 黄色）
- 校验不影响正常输入速度，无卡顿

---

#### R3.3 发送历史持久化

**问题**: `SendHistory` 数据仅存内存，重启清空。

**方案**:

1. 在 `SendHistory` 中增加 `save()` 和 `load()` 方法，通过 `SettingsManager` 持久化到 `embeddebug_settings.json`。
2. 保存 key: `serial/sendHistory`，值为 JSON 数组，每个元素: `{"text": "...", "isHex": true/false, "time": "ISO8601"}`。
3. 调用时机:
   - `save()`: 每次 `addEntry()` 后自动调用（防抖 5 秒，避免频繁写盘）
   - `load()`: MainWindow 初始化时调用
4. 持久化最多 50 条（与内存上限一致），超出时自动淘汰最旧的。
5. 持久化文件损坏时，静默忽略，从空历史开始。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R3.3-01 | 发送 "AT"、"AT+RST"、"AT+GMR" → 关闭 → 重新打开 | 自动补全列表包含这三条记录 |
| TC-R3.3-02 | 发送 60 条不同命令 → 关闭 → 重新打开 | 历史中只保留最新 50 条 |
| TC-R3.3-03 | 发送 "test" (文本) 和 "AA BB" (HEX) → 关闭 → 重新打开 | 两条记录的 isHex 标记正确 |
| TC-R3.3-04 | settings 文件被手动清空 | 应用正常启动，历史为空 |
| TC-R3.3-05 | 快速连续发送 20 条命令 | 写盘不阻塞 UI，防抖生效 |

**验收标准**:
- 发送历史在应用重启后完整保留
- 持久化操作不阻塞 UI
- 数据损坏时优雅降级

---

#### R3.4 快捷指令持久化与编辑对话框

**问题**: 快捷指令无持久化，无编辑对话框。

**方案**:

1. **持久化**: 在 `QuickCommandBar` 中增加 `save()` 和 `load()` 方法:
   - 保存 key: `serial/quickCommands`
   - 值为 JSON 数组: `[{"name": "复位", "data": "AA BB CC", "isHex": true}, ...]`
   - 调用时机: `addCommand()`/`clearCommands()` 后自动保存（防抖），MainWindow 初始化时 `load()`

2. **编辑对话框**: 新增 `QuickCommandEditDialog` 类（表现层）:
   - 列表视图: 显示所有指令，每行显示名称 + 数据预览 + HEX 标记
   - 操作按钮: 添加 / 编辑 / 删除 / 上移 / 下移 / 导入 / 导出
   - 编辑表单: 名称输入框 + 数据输入框 + HEX/文本切换 + HEX 实时预览
   - 确认/取消按钮
   - 对话框模态显示，由 `editRequested()` 信号触发

3. 编辑对话框保存后，调用 `QuickCommandBar::setCommands()` 刷新按钮栏。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R3.4-01 | 添加快捷指令 "复位" → 关闭 → 重新打开 | 指令按钮 "复位" 仍在 |
| TC-R3.4-02 | 点击 "Edit" 按钮 | 弹出编辑对话框，显示所有已有指令 |
| TC-R3.4-03 | 在编辑对话框中添加 "查询版本" 指令 → 保存 | 按钮栏新增 "查询版本" 按钮 |
| TC-R3.4-04 | 编辑已有指令的名称和数据 → 保存 | 按钮文字和数据同步更新 |
| TC-R3.4-05 | 删除一条指令 → 保存 | 按钮栏移除对应按钮 |
| TC-R3.4-06 | 调整指令顺序（上移/下移）→ 保存 | 按钮栏顺序同步更新 |
| TC-R3.4-07 | 点击快捷指令按钮 → 指令数据正确发送 | 数据格式和内容与编辑时一致 |
| TC-R3.4-08 | 添加 20+ 条指令 | 按钮栏自动换行或提供滚动，不溢出 |

**验收标准**:
- 快捷指令在应用重启后完整保留
- 编辑对话框提供完整的 CRUD 操作
- 编辑对话框 UI 符合 CLAUDE.md 现代化 UI 标准

---

### R4: 接收功能完善 (P1)

#### R4.1 DataStatistics 数据联动

**问题**: `DataStatistics::update(rxBytes, txBytes)` 可能未被调用，面板显示 "0 B"。

**方案**:

1. 在 `ConnectionController` 中，每次 `onDataReceived()` 被调用时，累加接收字节数到 `m_totalRxBytes`。
2. 在 `SendController` 中，每次 `sendAndRecord()` 成功时，累加发送字节数并发出 `dataSent(bytes)` 信号。
3. MainWindow 连接以下信号到 `DataStatistics::update(rxBytes, txBytes)`:
   - `ConnectionController::dataReceived` → 累加 rxBytes
   - `SendController::dataSent` → 累加 txBytes
   - 每次累加后调用 `DataStatistics::update(m_totalRxBytes, m_totalTxBytes)`
4. 连接建立时调用 `DataStatistics::reset()`，断开时停止更新。

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R4.1-01 | 连接串口，接收 1024 字节数据 | RX 显示 "1.0 KB"，速率实时更新 |
| TC-R4.1-02 | 发送 "AT\r\n" (4 字节) | TX 显示 "4 B" |
| TC-R4.1-03 | 持续收发数据 1 分钟 | 累计字节数持续增长，速率每秒更新 |
| TC-R4.1-04 | 断开连接 → 重新连接 | 统计面板重置为 0，持续时间重新计时 |
| TC-R4.1-05 | 高速接收 (115200 波特率满载) | 速率显示准确（约 11.5 KB/s），UI 无卡顿 |

**验收标准**:
- DataStatistics 面板实时显示正确的 RX/TX 字节数和速率
- 连接持续时间精确计时
- 重连后统计数据正确重置

---

#### R4.2 显示模式切换 UI 入口确认

**当前状态分析**: `TerminalWidget` 已实现 `setDisplayMode(DisplayMode)` 方法，支持 Text/Hex/Mixed/Decimal 四种模式。需确认 MainWindow 中有 UI 入口（如工具栏按钮或右键菜单）触发模式切换。

**方案**: 如果当前无 UI 入口，在终端区域右键菜单或工具栏中增加显示模式切换按钮组:
- 文本模式 (默认)
- HEX 模式
- 混合模式
- 十进制模式

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R4.2-01 | 切换到 HEX 模式 | 接收数据以 "AA BB CC" 格式显示 |
| TC-R4.2-02 | 切换到混合模式 | 接收数据以 "文本 | AA BB CC" 格式显示 |
| TC-R4.2-03 | 切换到十进制模式 | 接收数据以 "170 187 204" 格式显示 |
| TC-R4.2-04 | 切换回文本模式 | 接收数据以可读文本显示 |

**验收标准**:
- 用户可通过 UI 切换四种显示模式
- 模式切换即时生效，无需重连
- 模式切换不影响数据接收

---

#### R4.3 时间戳/方向前缀开关确认

**当前状态分析**: `TerminalWidget` 已实现 `setShowTimestamp(bool)` 和 `setShowDirectionPrefix(bool)` 方法。需确认 MainWindow 中有 UI 入口触发这两个开关。

**方案**: 如果当前无 UI 入口，在终端区域右键菜单或工具栏中增加:
- 时间戳开关（Toggle）
- 方向前缀开关（Toggle）

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R4.3-01 | 开启时间戳 | 每行数据前显示 "14:30:25.123" 格式时间戳 |
| TC-R4.3-02 | 关闭时间戳 | 时间戳消失，数据行恢复紧凑显示 |
| TC-R4.3-03 | 开启方向前缀 | 接收数据显示 "[RX:]"，发送数据显示 "[TX:]" |
| TC-R4.3-04 | 同时开启时间戳和方向前缀 | 每行显示 "14:30:25 [RX:] data" |

**验收标准**:
- 时间戳和方向前缀开关可通过 UI 独立控制
- 开关状态即时生效

---

### R5: 串口驱动检测增强 (P1)

#### R5.1 中文提示与可操作指引

**问题**: `SerialDriverDetector::driverStatusSummary()` 输出英文提示，缺少可操作指引。

**方案**:

1. 将 `driverStatusSummary()` 的所有输出改为中文:

| 场景 | 当前输出 | 改进后输出 |
|------|---------|-----------|
| 无端口 | "No serial port devices detected..." | "未检测到串口设备。\n请检查:\n1. USB 转串口适配器是否已连接\n2. 串口驱动是否已安装 (CH340/CP2102/FT232/PL2303)\n3. 设备是否已上电" |
| 检测到已知驱动 | "Detected serial drivers:\n..." | "已检测到串口驱动:\nCH340 (WCH CH340 Serial Adapter)\n\n可用端口: 2 个" |
| 有端口但无匹配 | "Serial ports detected but no known USB-serial driver matched..." | "检测到串口端口但未匹配已知 USB 转串口驱动。\n可用端口: COM3, COM4\n这些端口可能仍可使用（系统原生串口或未识别的适配器）" |

2. 在 `SerialConfigPanel` 中，驱动信息标签改为富文本 QLabel:
   - 检测到驱动时: 绿色图标 + 成功提示
   - 无端口时: 黄色图标 + 诊断提示 + "点击此处查看驱动安装指南" 链接
   - 点击链接打开系统默认浏览器跳转到驱动下载页面

3. 常见驱动下载链接（内置于应用中）:

| 驱动芯片 | 下载链接 |
|---------|---------|
| CH340/CH910 | http://www.wch.cn/downloads/CH340DriverEXE.html |
| CP2102/CP210x | https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers |
| FT232/FTDI | https://ftdichip.com/drivers/vcp-drivers/ |
| PL2303 | https://www.prolific.com.tw/US/ShowProduct.aspx?p_id=225&pcid=41 |

**测试用例**:

| 测试编号 | 操作 | 预期结果 |
|---------|------|---------|
| TC-R5.1-01 | 插入 CH340 设备 → 打开应用 | 驱动信息显示 "已检测到: CH340"，绿色图标 |
| TC-R5.1-02 | 无 USB 转串口设备 → 打开应用 | 显示黄色提示 + "点击此处查看驱动安装指南" |
| TC-R5.1-03 | 点击驱动安装指南链接 | 浏览器打开驱动下载页面 |
| TC-R5.1-04 | 有原生 COM 口但无 USB 转串口 | 显示 "检测到端口但未匹配已知驱动" 的中性提示 |
| TC-R5.1-05 | 插入新设备后点击刷新 | 驱动信息实时更新 |

**验收标准**:
- 所有驱动检测信息以中文显示
- 无驱动时有明确的可操作指引（链接可点击）
- 驱动检测状态有颜色编码（绿色=成功，黄色=警告）

---

## 接口设计

### 新增类

#### QuickCommandEditDialog (表现层)

```cpp
// serial/QuickCommandEditDialog.h

/**
 * @brief 快捷指令编辑对话框
 *
 * 提供快捷指令的 CRUD 界面:
 *   - 列表视图: 显示所有指令（名称 + 数据预览 + HEX 标记）
 *   - 编辑表单: 名称 + 数据 + HEX/文本模式
 *   - 操作: 添加 / 编辑 / 删除 / 上移 / 下移
 *   - 持久化: 确认后通过 SettingsManager 保存
 */
class QuickCommandEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit QuickCommandEditDialog(const QList<QuickCommand>& commands,
                                     QWidget* parent = nullptr);

    /** @brief 获取编辑后的指令列表 */
    QList<QuickCommand> resultCommands() const;

private:
    void setupUI();
    void updateList();
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

    QList<QuickCommand> m_commands;     ///< 当前指令列表
    QListWidget* m_listWidget;          ///< 指令列表视图
    // ... 表单控件
};
```

### 新增方法

#### SerialConfigPanel

```cpp
/** @brief 波特率输入校验槽函数 */
void onBaudRateTextChanged(const QString& text);
```

#### ConnectionController

```cpp
/**
 * @brief 启动自动重连
 * @param params 上一次成功的连接参数
 * @param maxRetries 最大重试次数 (默认 10)
 */
void startAutoReconnect(const QVariantMap& params, int maxRetries = 10);

/** @brief 取消自动重连 */
void cancelAutoReconnect();

signals:
    /** @brief 自动重连状态变化 (重试次数 / 最大次数) */
    void reconnectProgress(int attempt, int maxAttempts);
```

#### SendHistory

```cpp
/** @brief 将历史保存到 SettingsManager (防抖) */
void save();

/** @brief 从 SettingsManager 加载历史 */
void load();
```

#### QuickCommandBar

```cpp
/** @brief 将指令保存到 SettingsManager (防抖) */
void save();

/** @brief 从 SettingsManager 加载指令 */
void load();
```

#### SerialDriverDetector

```cpp
/**
 * @brief 获取常见驱动的下载链接列表
 * @return QList<QPair<驱动名, URL>>
 */
static QList<QPair<QString, QString>> driverDownloadLinks();
```

### 修改方法

#### SerialConnection

```cpp
// open() 增加错误码到中文消息的映射
// onError() 增加 close() 确保端口完全释放
```

---

## 依赖的公共组件

| 组件 | 用途 |
|------|------|
| `SettingsManager` | 串口配置/发送历史/快捷指令的持久化存储 |
| `HexConverter` | HEX 输入校验和格式转换 |
| `TerminalModel` | RX/TX 字节统计，提供给 DataStatistics |
| `ConnectionManager` | 自动重连时创建新的 SerialConnection 实例 |

---

## 设计模式

| 模式 | 应用场景 |
|------|---------|
| 状态模式 | 自动重连状态机（Waiting → Retrying → Connected → Abandoned） |
| 观察者模式 | 连接状态变化通过信号/槽通知 UI 更新 |
| 防抖模式 | 持久化写操作的节流（避免频繁写盘） |

---

## 影响范围

### 修改文件

| 文件 | 修改内容 |
|------|---------|
| `serial/SerialConfigPanel.h/cpp` | 波特率校验、连接状态细粒度显示 |
| `connection/SerialConnection.cpp` | 错误码中文映射、onError 增强 |
| `core/ConnectionController.h/cpp` | 自动重连、连接超时、配置持久化调用 |
| `core/SendController.h/cpp` | 定时发送 UI 控件、HEX 实时校验 |
| `serial/SendHistory.h/cpp` | 持久化 save/load 方法 |
| `serial/QuickCommandBar.h/cpp` | 持久化 save/load 方法 |
| `serial/DataStatistics.h/cpp` | 联动修复确认 |
| `serial/SerialDriverDetector.h/cpp` | 中文提示、驱动下载链接 |
| `core/MainWindow.cpp` | 信号连接: DataStatistics 联动、配置恢复、显示模式切换 UI |

### 新增文件

| 文件 | 内容 |
|------|------|
| `serial/QuickCommandEditDialog.h/cpp` | 快捷指令编辑对话框 |

---

## 验收标准总表

### P0 验收（必须全部通过才能 commit）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P0-01 | 波特率校验 | 输入非法值时有红色边框反馈，currentBaudRate() 永不返回 0 |
| AC-P0-02 | 配置持久化 | 重启应用后，上次成功连接的参数自动恢复 |
| AC-P0-03 | 连接状态同步 | 任何连接状态变化，面板按钮文字和控件可用性正确反映 |
| AC-P0-04 | 错误友好提示 | 所有连接错误以中文展示，附具体解决建议 |
| AC-P0-05 | 自动重连 | USB 拔出后自动重连，成功后恢复正常工作状态 |
| AC-P0-06 | 定时发送 UI | 用户可通过 UI 控制定时发送的开启/关闭/间隔 |
| AC-P0-07 | HEX 实时校验 | HEX 模式下输入非法字符时有即时视觉反馈 |
| AC-P0-08 | 发送历史持久化 | 重启应用后，发送历史自动恢复到自动补全列表 |
| AC-P0-09 | 快捷指令持久化 | 重启应用后，快捷指令按钮完整保留 |
| AC-P0-10 | 快捷指令编辑 | 有完整的编辑对话框，支持添加/编辑/删除/排序 |

### P1 验收（本迭代应完成，如时间不足可延后）

| 编号 | 验收项 | 通过条件 |
|------|--------|---------|
| AC-P1-01 | DataStatistics 联动 | 面板实时显示正确的 RX/TX 字节数和速率 |
| AC-P1-02 | 显示模式切换 | 有 UI 入口切换 Text/Hex/Mixed/Decimal 四种模式 |
| AC-P1-03 | 时间戳/方向前缀 | 有 UI 入口控制时间戳和方向前缀开关 |
| AC-P1-04 | 驱动检测中文 | 所有驱动检测信息以中文展示，无驱动时有可操作链接 |

---

## 实现优先级排序

按依赖关系和用户价值排序的实现顺序:

```
第一批（核心可用性，无依赖）:
  R1.1 波特率校验 ─────────── 30 分钟
  R1.4 错误友好提示 ────────── 45 分钟
  R3.2 HEX 实时校验 ────────── 30 分钟

第二批（稳定性，依赖第一批）:
  R2.2 错误恢复 ───────────── 60 分钟
  R2.1 自动重连 ───────────── 90 分钟

第三批（持久化，无互相依赖）:
  R1.2 配置持久化 ─────────── 45 分钟
  R3.3 发送历史持久化 ──────── 45 分钟
  R3.4 快捷指令持久化 + 编辑 ── 120 分钟

第四批（UI 完善，依赖前面）:
  R1.3 连接状态双向同步 ────── 45 分钟
  R3.1 定时发送 UI ────────── 90 分钟

第五批（接收侧，可并行）:
  R4.1 DataStatistics 联动 ── 30 分钟
  R4.2 显示模式切换 UI ────── 30 分钟
  R4.3 时间戳/方向前缀 UI ──── 20 分钟

第六批（驱动增强，独立）:
  R5.1 中文提示 + 链接 ────── 45 分钟
```

**预估总工作量**: 约 11.5 小时

---

## 风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| 自动重连在端口资源未完全释放时重试失败 | 重连成功率低 | 每次重试前先 close() 旧端口，使用指数退避增加间隔 |
| SettingsManager 并发写入冲突 | 数据损坏 | 所有持久化操作在主线程执行，使用防抖合并写入 |
| QIntValidator 限制波特率范围 | 用户无法输入极端波特率 | 范围设为 300 ~ 12000000，覆盖所有已知硬件 |
| 快捷指令编辑对话框实现复杂 | 超出单次 commit 工作量 | 先实现基础 CRUD，导入/导出功能延后 |
