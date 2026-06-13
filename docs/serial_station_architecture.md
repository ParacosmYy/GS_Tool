# Serial Station C++ 架构约束

> 本文档定义串口上位机后续重构的 C++/Qt 目标边界。它参考 app/ui/core/protocols/services/workers 的分层方式，但落地形式必须符合当前 EmbedDebug 的 C++17、Qt Widgets、CMake、QTest 约束。
> 目标是先把“串口框架层”和“业务协议层”切开，再让后续 AI 或人工迭代只在明确模块边界内修改。

---

## 一、目标目录

Serial Station 作为新的独立 app 模块落地，建议使用 `src/apps/serial_station/`。不要把新串口工站继续塞进现有 `MainWindow`、`PanelManager` 或 `src/serial/` 的旧耦合结构里。

```text
src/apps/serial_station/
  SerialStationApp.h/.cpp
  SerialStationWindow.h/.cpp
  SerialStationController.h/.cpp

  SerialStationModels.h
  SerialStationConstants.h
  SerialStationConfig.h/.cpp

  ui/
    SerialMainPanel.h/.cpp
    SerialPortPanel.h/.cpp
    SerialProtocolPanel.h/.cpp
    SerialLogPanel.h/.cpp
    SerialCommandPanel.h/.cpp
    SerialStatusBar.h/.cpp

  core/
    SerialPort.h/.cpp
    SerialManager.h/.cpp
    SerialSession.h/.cpp
    SerialDispatcher.h/.cpp
    SerialCodec.h/.cpp
    SerialError.h

  protocols/
    ISerialProtocol.h
    SerialProtocolRegistry.h/.cpp
    SerialProtocolEvent.h

    modbus_rtu/
      ModbusRtuProtocol.h/.cpp
      ModbusRtuFrame.h/.cpp
      ModbusRtuCommand.h/.cpp
      ModbusRtuParser.h/.cpp

    custom_md/
      CustomMdProtocol.h/.cpp
      CustomMdFrame.h/.cpp
      CustomMdCommand.h/.cpp
      CustomMdParser.h/.cpp

    ascii_text/
      AsciiTextProtocol.h/.cpp
      AsciiTextCommand.h/.cpp
      AsciiTextParser.h/.cpp

  services/
    SerialLogService.h/.cpp
    SerialExportService.h/.cpp
    SerialReplayService.h/.cpp
    DeviceProfileService.h/.cpp

  workers/
    SerialReaderWorker.h/.cpp
    SerialCommandWorker.h/.cpp
```

测试放在仓库根目录 `tests/serial_station/`，不要放入生产源码目录：

```text
tests/serial_station/
  test_serial_manager.cpp
  test_serial_dispatcher.cpp
  test_serial_protocol_registry.cpp
  test_modbus_rtu_protocol.cpp
  test_custom_md_protocol.cpp
  test_ascii_text_protocol.cpp
```

---

## 二、分层职责

| 层 | 路径 | 职责 | 禁止事项 |
|----|------|------|----------|
| App 入口层 | `SerialStationApp`, `SerialStationWindow` | 工站装配、生命周期、顶层窗口或面板入口 | 不写协议解析、串口帧拼装、文件导出 |
| 控制层 | `SerialStationController` | 把 UI 事件转成 core/protocol/services 调用，汇总状态再更新 UI | 不持有协议解析细节，不写串口读写循环 |
| UI 层 | `ui/` | 显示状态、收集用户操作、发出 Qt signal | 不直接调用 `SerialManager`，不 include 具体协议目录 |
| 串口核心层 | `core/` | `QSerialPort` 薄封装、打开/关闭/发送/接收、会话状态、dispatcher、codec、错误类型 | 不关心 Modbus/custom/ascii 等具体协议 |
| 协议层 | `protocols/` | 命令构建、帧定义、流式解析、协议内部状态 | 不 include UI，不写文件，不改公共串口收发逻辑 |
| 服务层 | `services/` | 日志、导出、回放、设备档案等非串口核心能力 | 不直接操作 QWidget，不直接解析具体协议字节流 |
| Worker 层 | `workers/` | 后台读取、发送队列、耗时任务 | 不操作 QWidget，只通过 signal/slot 回主线程 |

---

## 二-A、Serial Station 交付状态

Serial Station 的功能状态必须拆成三轴，不允许只写“UART 已完成”或“协议已完成”。

| 能力 | 工程完成最低要求 | 用户完成最低要求 | 设备完成最低要求 |
|------|------------------|------------------|------------------|
| UART 配置 | `SerialPortConfig`、`SerialPortPanel`、Controller 接入、CMake、QTest | 用户能选择或手动输入端口，看到 115200 8N1 等摘要，连接失败有明确反馈 | fake/虚拟串口/真实 COM 至少一种验证；真实可用必须记录 COM 和设备 |
| UART 发送 | `SerialCodec`、`SerialManager::send`、Controller 错误路径测试 | 用户能输入 ASCII/HEX/协议命令并看到 TX 日志或失败原因 | 回环、虚拟串口或真实设备验证发送字节 |
| UART 接收 | `SerialPort::readyRead`、`SerialDispatcher`、协议 feed、RX 日志测试 | 用户能看到 RX 日志、计数、半包/错误提示 | 回环、虚拟串口或真实设备验证接收字节 |
| 协议支持 | `ISerialProtocol` 实现、registry 注册、命令构建和流式解析 QTest | 用户能在工作台选择协议并按协议发送/查看解析结果 | 协议设备或协议样本验证；纯 parser 测试只算 `D1` |
| 日志/导出/回放 | service 层测试、Controller/Window 闭环测试 | 用户能点击导出/回放入口并得到文件或预览反馈 | 不要求真实设备，但真实日志样本可提升验证等级 |

状态规则：

- 只有生产文件存在或进入 CMake，最多只能说明 `E3`。
- 有 QTest 通过，最多说明 `E4`，不能自动提升用户状态。
- 没有从主程序或明确工站入口可达，用户状态不得高于 `U1`。
- 没有真实/虚拟/替身串口验证，设备状态不得高于 `D1`。
- README 宣传 Serial Station 时必须写清当前成熟度，不能把 preview 能力写成稳定硬件调试能力。

---

## 三、硬性边界

1. 新增协议只能放在 `src/apps/serial_station/protocols/<protocol_name>/`。
2. 串口连接、线程、收发缓冲只能修改 `core/` 和 `workers/`。
3. UI 只能通过 Qt signal 调用 `SerialStationController`，不允许直接调用 `SerialManager` 或具体 protocol。
4. 协议模块不能 include `ui/`、`SerialStationWindow` 或任何 QWidget 派生面板。
5. `core/` 不能 include `protocols/modbus_rtu/`、`protocols/custom_md/` 等具体协议目录，只能依赖 `ISerialProtocol` 和 `SerialProtocolRegistry`。
6. `SerialProtocolRegistry` 可以注册具体协议，但 registry 对外只暴露 `ISerialProtocol*` 或工厂函数。
7. 每新增一个协议，必须增加对应命令构建测试和流式解析测试。
8. 不允许为了一个协议修改公共串口收发逻辑，除非先在 PRD 或架构说明中证明这是框架缺陷。
9. 日志、导出、回放只放在 `services/`，UI 和协议层不得直接写文件。
10. `SerialStationController.cpp` 超过 300 行时必须拆分子 controller 或 service；超过 500 行不允许继续追加功能。
11. `SerialStationWindow` 只装配和连接信号，不做协议选择逻辑、导出格式判断以外的业务决策；复杂逻辑必须下沉 controller/service。
12. 协议选择 UI 只能传协议名或用户意图，不持有具体协议对象。
13. `SerialProtocolRegistry` 是具体协议注册的唯一集中点；其他层不得直接 new 具体协议类。
14. 用户状态提升到 `U3` 前，必须证明端口配置、连接、发送、接收、日志反馈在一个工作台路径内闭环。
15. 设备状态提升到 `D3/D4` 前，必须写明虚拟串口或真实设备验证方式。

---

## 四、协议接口

所有协议必须实现 `ISerialProtocol`。接口只使用 Qt Core 类型和 serial_station 内部轻量事件类型。

```cpp
class ISerialProtocol {
public:
    virtual ~ISerialProtocol() = default;

    virtual QString name() const = 0;
    virtual QByteArray buildCommand(const QString& command,
                                    const QVariantMap& params) const = 0;
    virtual QVector<SerialProtocolEvent> feed(const QByteArray& data) = 0;
    virtual void reset() = 0;
};
```

### 事件返回要求

`feed()` 返回结构化事件，不返回 UI 文案，不直接更新控件：

```cpp
struct SerialProtocolEvent {
    QString type;          // frame, error, log, measurement
    QString protocolName;
    QVariantMap payload;
    QByteArray raw;
};
```

UI 文案由 `SerialStationController` 或 UI 层根据事件转换。协议层只描述事实。

---

## 五、典型调用链

用户点击“发送读取版本”：

```text
SerialCommandPanel::readVersionClicked()
  -> SerialStationController::sendCommand("read_version", params)
  -> current ISerialProtocol::buildCommand(...)
  -> SerialManager::send(frame)
  -> SerialReaderWorker 收到 QByteArray
  -> SerialDispatcher::feed(bytes)
  -> current ISerialProtocol::feed(bytes)
  -> SerialStationController 处理 SerialProtocolEvent
  -> SerialLogPanel / SerialStatusBar 更新显示
```

关键点：

- UI 只发意图，不拼帧。
- core 只负责 bytes 传输和分发，不理解业务命令。
- 协议只处理命令和帧，不更新 UI。
- controller 是 UI 与业务之间唯一协调入口。

---

## 六、测试准入

| 改动类型 | 必须测试 |
|----------|----------|
| 新增协议 | `tests/serial_station/test_<protocol>_protocol.cpp` 覆盖命令构建、完整帧解析、半包/粘包、异常校验 |
| 修改 `SerialManager` | `test_serial_manager.cpp` 覆盖打开、关闭、发送、错误传播 |
| 修改 `SerialDispatcher` | `test_serial_dispatcher.cpp` 覆盖协议切换、增量 feed、异常隔离 |
| 修改 `SerialProtocolRegistry` | `test_serial_protocol_registry.cpp` 覆盖注册、查找、默认协议、重复名称处理 |
| 修改日志/导出/回放 | service 级测试覆盖文件格式和错误路径 |

测试不得依赖真实 COM 口。需要串口输入时，使用 fake serial port、内存缓冲或可注入的 `ISerialPort` 测试替身。

### 6.1 验证等级

| 等级 | Serial Station 含义 | 可接受证据 |
|------|----------------------|------------|
| `D1` | 纯单测 | 协议 parser、service、controller QTest |
| `D2` | 替身验证 | fake serial port、内存回环、可注入 I/O 替身 |
| `D3` | 虚拟设备验证 | com0com、com2tcp、虚拟串口对或脚本化回环 |
| `D4` | 真实设备验证 | 真实 COM 口、USB-UART、MCU 开发板、J-Link/RTT 等记录 |

涉及 UART 的 PRD 如果没有至少 `D2` 方案，只能做工程骨架或 UI 预览；不能标为真实调试可用。

---

## 七、AI 迭代规则

1. 每次任务必须先声明本次改动属于 `ui`、`controller`、`core`、`protocols`、`services`、`workers` 中哪一层。
2. 单次任务默认只允许修改一个层；跨层修改必须先写清原因和调用链。
3. 新增协议时，只能新增或修改 `protocols/<protocol_name>/` 和对应测试，不能顺手改 UI/core。
4. 修改 UI 时，不能把协议解析或 `QByteArray` 拼接写进控件槽函数。
5. 修改 core 时，不能 include 具体协议头文件。
6. 修改 service 时，不能直接触碰串口线程。
7. 任何“为了快速实现”而跨过 controller 的 UI 到 core/protocol 直连，必须退回。
8. 如果现有结构无法满足需求，先更新本架构文档或写迁移 PRD，再改代码。
9. 每次任务必须声明本轮三轴目标，例如 `E4 + U2 + D1`；收口时写实际状态。
10. 子 Agent 默认只改一个层；跨层任务必须由主 Agent 拆成顺序步骤。
11. 同一轮中 `CMakeLists.txt`、README、QSS、SerialStationController、SerialStationWindow 由单一负责人修改。
12. 任何新增 UI 控件必须设置 `objectName`，用户可见文字必须 `tr()`，并在三套主题中确认状态样式。

---

## 八、迁移策略

1. 先建立 `src/apps/serial_station/` 新工站骨架，不直接重写现有 `src/serial/` 和 `src/protocol/`。
2. 先落 `ISerialProtocol`、`SerialProtocolRegistry`、`SerialCodec`、`SerialDispatcher`，再接 UI。
3. 第一批协议只保留最小可用集合：`ascii_text`、`modbus_rtu`、`custom_md`。
4. `SerialPort` 作为 `QSerialPort` 薄封装，避免 pyserial 或平台细节这类概念污染 UI。
5. 日志、导出、回放先做 service，禁止散落到控件槽函数。
6. 新工站闭环稳定后，再考虑把现有 `src/serial/`、`src/protocol/` 的能力迁移或适配进来。
7. 迁移优先级按用户闭环排序：入口可达 → UART 配置 → 连接/断开 → 发送 → 接收 → 协议选择 → 日志导出 → 回放/高级服务。
8. 旧 `src/serial/` 的能力迁入前必须先列出复用点和废弃点，不能复制第二套 CRC、Hex、日志、端口扫描或协议解析。
9. 每完成一个迁移阶段，都要更新三轴状态；没有设备验证的阶段不得把设备状态提升。

---

## 九、审查清单

每次 serial_station 相关 PR 或 commit 前确认：

- [ ] UI 没有 include `core/SerialManager.h` 或具体协议头文件。
- [ ] `core/` 没有 include `protocols/<具体协议名>/...`。
- [ ] 协议目录没有 include `ui/`、`services/` 或 QWidget。
- [ ] 新协议有命令构建测试和 parser/feed 测试。
- [ ] Worker 没有直接更新 UI。
- [ ] 文件写入只发生在 `services/`。
- [ ] controller 只做协调，没有塞入协议解析状态机。
- [ ] 新增公共逻辑没有复制到多个协议目录。
- [ ] 本轮工程/用户/设备状态已经写清，且未验证项没有被标为完成。
- [ ] 涉及 UART 主流程时，连接、发送、接收、日志反馈至少有替身或明确未验证说明。
- [ ] 并行开发时，本轮没有多个 Agent 同时修改同一锁定文件。
