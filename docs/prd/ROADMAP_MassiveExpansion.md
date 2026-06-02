# EmbedDebug 大规模扩展路线图 (Massive Expansion PRD)

> 状态: 规划中 | 版本: v1.0 | 基于: ROADMAP_FutureFeatures 已交付特性
> 目标: 将 EmbedDebug 从"串口调试工具"升级为"全球最全面的嵌入式调试助手"
> 前置条件: F1~F11 全部特性已规划或部分交付, 四层架构稳固, IConnection 接口成熟

---

## 目录

1. [扩展总览与优先级矩阵](#一扩展总览与优先级矩阵)
2. [F12: Bluetooth/BLE 调试](#f12-bluetoothble-调试)
3. [F13: CAN/CAN-FD 总线](#f13-cancan-fd-总线)
4. [F14: MQTT 客户端](#f14-mqtt-客户端)
5. [F15: TCP/UDP 增强](#f15-tcpudp-增强)
6. [F16: SPI/I2C 桥接](#f16-spii2c-桥接)
7. [F17: WebSocket 客户端](#f17-websocket-客户端)
8. [F18: Modbus 主站/从站](#f18-modbus-主站从站)
9. [F19: Protobuf/FlatBuffers 解码](#f19-protobufflatbuffers-解码)
10. [F20: USB 调试](#f20-usb-调试)
11. [F21: 正则终端过滤器](#f21-正则终端过滤器)
12. [F22: 校验和计算器](#f22-校验和计算器)
13. [F23: ASCII/表格转换器](#f23-ascii表格转换器)
14. [F24: 时间戳分析器](#f24-时间戳分析器)
15. [F25: 数据包构建器](#f25-数据包构建器)
16. [F26: 设备注册表](#f26-设备注册表)
17. [依赖关系图](#依赖关系图)
18. [里程碑规划](#里程碑规划)
19. [代码量估算](#代码量估算)

---

## 一、扩展总览与优先级矩阵

### 优先级定义

| 级别 | 含义 | 交付窗口 |
|------|------|---------|
| **P0** | 核心差异化能力, 竞品没有或做得差 | MS1 立即启动 |
| **P1** | 完善通信层覆盖, 填补主流协议空白 | MS2 中近期交付 |
| **P2** | 高级协议支持, 提升专业度 | MS3 中期交付 |
| **P3** | 辅助工具集, 提升日常调试效率 | MS4 远期交付 |

### 特性矩阵

| ID | 特性名称 | 优先级 | 模块 | 预估代码量 | 依赖 | 竞品参考 |
|----|---------|--------|------|-----------|------|---------|
| F12 | Bluetooth/BLE 调试 | **P0** | `connection/ble/` | ~2200行 | F15(SSL) | nRF Connect, LightBlue |
| F13 | CAN/CAN-FD 总线 | **P0** | `connection/can/` | ~2400行 | 无 | CANoe, BUSMASTER, SavvyCAN |
| F14 | MQTT 客户端 | **P1** | `connection/mqtt/` | ~1600行 | F17(WS) | MQTTX, MQTT Explorer |
| F15 | TCP/UDP 增强 | **P1** | `connection/tcp/` | ~1000行 | 无 | Hercules, SocketTester |
| F16 | SPI/I2C 桥接 | **P1** | `connection/spi_i2c/` | ~1400行 | 无 | Aardvark, Bus Pirate |
| F17 | WebSocket 客户端 | **P1** | `connection/ws/` | ~800行 | 无 | Postman, wscat |
| F18 | Modbus 主站/从站 | **P2** | `protocol/modbus/` | ~1800行 | F2(协议引擎) | ModScan, Simply Modbus |
| F19 | Protobuf/FlatBuffers 解码 | **P2** | `protocol/protobuf/` | ~1200行 | F2(协议引擎) | Protobuf Inspector |
| F20 | USB 调试 | **P2** | `connection/usb/` | ~1500行 | 无 | Wireshark USB, USBlyzer |
| F21 | 正则终端过滤器 | **P3** | `terminal/filter/` | ~600行 | 无 | MobaXterm, CoolTerm |
| F22 | 校验和计算器 | **P3** | `utils/checksum/` | ~500行 | 无 | CRC Calculator Online |
| F23 | ASCII/表格转换器 | **P3** | `utils/converter/` | ~400行 | 无 | CyberChef, HexEd.it |
| F24 | 时间戳分析器 | **P3** | `utils/timestamp/` | ~300行 | 无 | Epoch Converter |
| F25 | 数据包构建器 | **P3** | `utils/packet/` | ~700行 | F22(校验和) | Packet Sender, Docklight |
| F26 | 设备注册表 | **P3** | `core/device/` | ~600行 | 无 | VS Code Device Profile |

### 总代码量概览

| 优先级 | 特性数 | 总代码行 | 新增文件数 |
|--------|--------|---------|-----------|
| P0 | 2 | ~4600 | ~32 |
| P1 | 4 | ~4800 | ~34 |
| P2 | 3 | ~4500 | ~30 |
| P3 | 5 | ~2600 | ~22 |
| **合计** | **14** | **~16500** | **~118** |

---

## F12: Bluetooth/BLE 调试

> Priority: P0 | Module: `connection/ble/` | Estimate: ~2200行 | Dependencies: F15(SSL for BLE Secure)

### 动机

BLE 已经成为嵌入式设备最常见的无线通信方式之一。IoT 传感器、智能手表、蓝牙耳机、工业遥测设备, 几乎所有消费和工业级嵌入式产品都在用 BLE。然而目前市面上的 BLE 调试工具(nRF Connect, LightBlue)专注于手机平台, PC 端缺少一个集成了串口调试、波形显示、协议解析的一站式工具。

EmbedDebug 已经有成熟的 IConnection 接口和终端/波形基础设施。BLE 连接作为 IConnection 的新实现, 可以无缝接入现有的终端显示、波形绘制、协议解析和数据录制功能。开发者在调试 BLE 设备时不再需要切换三个工具, 直接在 EmbedDebug 里完成从物理层到应用层的全栈调试。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F12-R1 | BLE 扫描: 发现周围 BLE 设备, 显示设备名/RSSI/广播数据/服务UUID | P0 |
| F12-R2 | GATT 服务/特征浏览器: 连接后浏览所有 Service 和 Characteristic, 显示 UUID/属性(读/写/通知) | P0 |
| F12-R3 | 特征值读写: 读取/写入 Characteristic 值, 支持 HEX 和文本模式 | P0 |
| F12-R4 | BLE 通知订阅: 订阅 Notify/Indicate 特征, 实时接收数据并接入终端/波形管道 | P0 |
| F12-R5 | AT 命令控制台: 经典蓝牙(HC-05/HC-06/BF10)的 AT 指令交互界面 | P1 |
| F12-R6 | BLE 广播嗅探器: 被动监听周围 BLE 广播包, 解析 ADV 数据 | P1 |
| F12-R7 | HCI 监控: 捕获 HCI 命令/事件, 用于蓝牙协议栈调试 | P2 |
| F12-R8 | BLE 连接参数配置: Connection Interval/Slave Latency/Supervision Timeout | P2 |
| F12-R9 | GATT 配置文件导入: 从 Bluetooth SIG 导入标准 Service UUID 定义 | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `BleConnection` | `connection/ble/BleConnection.h/cpp` | 基础设施层 | IConnection 实现, BLE 设备连接和数据读写 |
| `BleScanner` | `connection/ble/BleScanner.h/cpp` | 基础设施层 | BLE 设备扫描, RSSI 采集, 广播数据解析 |
| `GattServiceModel` | `connection/ble/GattServiceModel.h/cpp` | 数据层 | GATT 服务/特征的树形数据模型 |
| `GattBrowserWidget` | `connection/ble/GattBrowserWidget.h/cpp` | 表现层 | GATT 服务浏览器 UI, 树形视图 |
| `BleConfigPanel` | `connection/ble/BleConfigPanel.h/cpp` | 表现层 | BLE 连接配置面板 |
| `BleAdvSniffer` | `connection/ble/BleAdvSniffer.h/cpp` | 基础设施层 | BLE 广播包被动监听和解析 |
| `AtCommandConsole` | `connection/ble/AtCommandConsole.h/cpp` | 表现层 | 经典蓝牙 AT 指令交互界面 |
| `BleNativeBackend` | `connection/ble/BleNativeBackend.h/cpp` | 基础设施层 | Windows BLE API (WinRT) 封装 |
| `BleAdvPacketParser` | `connection/ble/BleAdvPacketParser.h/cpp` | 数据层 | BLE 广播包字段解析(Flags/UUID/Name/TX Power/Manufacturer Data) |
| `HciMonitor` | `connection/ble/HciMonitor.h/cpp` | 基础设施层 | HCI 层命令/事件捕获 |

### 设计要点

- `BleConnection` 实现 `IConnection` 接口, `write()` 写入指定 Characteristic, `dataReceived` 对应 Notify 回调
- Windows 平台使用 WinRT `Windows::Devices::Bluetooth` API, 通过 C++/WinRT 投影调用
- `BleNativeBackend` 封装平台差异, 未来可扩展 Linux BlueZ 后端
- GATT 浏览器使用 `QTreeView` + 自定义 `GattServiceModel` (继承 `QAbstractItemModel`)
- AT 命令控制台复用现有 `TerminalWidget`, 通过经典蓝牙 SPP 连接
- 广播嗅探器独立于连接, 不需要配对, 只监听 ADV_IND/ADV_NONCONN_IND 包

### 验收标准

1. 可扫描并发现周围 BLE 设备, 显示名称和 RSSI
2. 连接 BLE 设备后可浏览 GATT 服务和特征列表
3. 可读写 Characteristic 值, HEX 和文本模式切换正常
4. 订阅 Notify 特征后数据实时流入终端和波形通道
5. J-Link 未安装/蓝牙适配器不存在时, 应用不崩溃, BLE 选项显示提示
6. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F13: CAN/CAN-FD 总线

> Priority: P0 | Module: `connection/can/` | Estimate: ~2400行 | Dependencies: 无

### 动机

CAN 总线是汽车电子、工业控制、电机驱动领域最主流的通信总线。EmbedDebug 目前只能通过串口/TCP 间接调试 CAN 设备(比如通过 CAN-to-USB 适配器的虚拟串口)。直接支持 CAN 通信, 意味着 EmbedDebug 将从"串口调试器"正式进入"嵌入式总线分析器"领域。

CANoe 和 BUSMASTER 是当前的行业标准, 但它们价格昂贵(CANoe 单个 License 几万欧元)且界面臃肿。SavvyCAN 开源免费但功能分散。EmbedDebug 的优势在于: CAN 总线数据可以直接流入波形引擎、协议解析引擎、触发器系统, 形成从物理层到应用层的完整调试闭环。

DBC (Database CAN) 文件是汽车行业的标准信号定义格式。导入 DBC 文件后, 原始 CAN 帧可以被解码为人类可读的物理值(如"引擎转速 3200 RPM"), 这比手动查 DBC 文档再心算换算效率高出一个数量级。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F13-R1 | CAN 适配器支持: SocketCAN (Linux), PCAN-USB, CANable/SLCAN, Vector VN16xx | P0 |
| F13-R2 | CAN 帧监视器: 实时显示总线上的 CAN 帧(ID/DLC/Data/时间戳/方向) | P0 |
| F13-R3 | CAN 帧发送: 手动/周期/响应式发送标准/扩展帧, 支持 CAN-FD (64字节 payload) | P0 |
| F13-R4 | DBC 文件导入: 解析 DBC 文件, 提取 Message/Signal 定义, 支持多文件加载 | P0 |
| F13-R5 | 信号提取: 根据DBC定义从 CAN 帧中提取物理信号值(含因子/偏移/单位) | P0 |
| F13-R6 | 信号值映射到波形通道: DBC 信号自动绑定到 ChartModel 通道 | P1 |
| F13-R7 | CAN 总线统计: 总线负载率/帧计数/错误帧率, 实时曲线显示 | P1 |
| F13-R8 | 过滤器: 按 CAN ID 范围/DBC Message 名称过滤接收帧 | P1 |
| F13-R9 | CAN 帧回放: 从录制文件(ASC/BLF/TRC)回放 CAN 数据流 | P2 |
| F13-R10 | CAN-FD 支持: FD 帧(BRS/ESI 标志), 64 字节 payload | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `CanConnection` | `connection/can/CanConnection.h/cpp` | 基础设施层 | IConnection 实现, CAN 总线通信 |
| `CanAdapter` | `connection/can/CanAdapter.h/cpp` | 基础设施层 | CAN 适配器抽象(位率/模式/过滤器) |
| `PcanAdapter` | `connection/can/PcanAdapter.h/cpp` | 基础设施层 | PCAN-USB 适配器驱动封装 |
| `SlcanAdapter` | `connection/can/SlcanAdapter.h/cpp` | 基础设施层 | SLCAN/Lawicel 协议适配器(通过串口) |
| `CanFrameParser` | `connection/can/CanFrameParser.h/cpp` | 数据层 | CAN 帧的解析和构建 |
| `DbcParser` | `connection/can/DbcParser.h/cpp` | 数据层 | DBC 文件词法/语法分析, 提取 Message/Signal |
| `DbcDatabase` | `connection/can/DbcDatabase.h/cpp` | 数据层 | DBC 消息/信号数据库, 查询接口 |
| `CanSignalExtractor` | `connection/can/CanSignalExtractor.h/cpp` | 数据层 | 从 CAN 帧提取 DBC 定义的物理信号值 |
| `CanMonitorWidget` | `connection/can/CanMonitorWidget.h/cpp` | 表现层 | CAN 帧监视器列表视图 |
| `CanSendWidget` | `connection/can/CanSendWidget.h/cpp` | 表现层 | CAN 帧发送面板(ID/DLC/Data 编辑) |
| `CanConfigPanel` | `connection/can/CanConfigPanel.h/cpp` | 表现层 | CAN 适配器选择和位率配置 |
| `DbcBrowserWidget` | `connection/can/DbcBrowserWidget.h/cpp` | 表现层 | DBC 消息/信号浏览器树形视图 |
| `CanStatsWidget` | `connection/can/CanStatsWidget.h/cpp` | 表现层 | 总线统计面板 |

### 设计要点

- `CanConnection` 实现 `IConnection`, `write()` 发送 CAN 帧, `dataReceived` 携带解析后的 CAN 帧
- CAN 帧通过 `CanFrame` 结构体(QVariantMap 编码)在管道中传递
- 适配器层抽象为 `CanAdapter` 接口, 统一 open/close/send/receive
- SLCAN 适配器复用串口连接(`SerialConnection`), 通过 AT 指令控制 CAN 模式
- DBC 解析器采用两阶段: 词法分析 → AST 构建, 参考 CAN DBC 3.0 规范
- 信号提取支持 Intel/Motorola 字节序, 因子(factor)*原始值 + 偏移(offset)
- PCAN 驱动通过 `QLibrary` 动态加载 `PCANBasic.dll`, 不安装时优雅降级
- CAN 帧数据可流入 `ProtocolEngine` 做上层协议解析(如 UDS/ISO-TP)

### 验收标准

1. PCAN-USB 适配器可连接并发送/接收 CAN 帧
2. SLCAN 适配器通过串口正确控制 CANable 设备
3. DBC 文件导入后显示消息/信号树, 解码值正确
4. 信号值可绑定到波形通道实时显示
5. CAN ID 过滤器生效, 总线统计实时更新
6. 适配器未连接/驱动未安装时应用正常, 不崩溃
7. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F14: MQTT 客户端

> Priority: P1 | Module: `connection/mqtt/` | Estimate: ~1600行 | Dependencies: F17(WebSocket 可选传输层)

### 动机

MQTT 是 IoT 领域的事实标准协议。嵌入式设备(ESP32/STM32+WiFi/树莓派)几乎都通过 MQTT 上报传感器数据和接收控制指令。开发者在调试 MQTT 设备时, 需要一个客户端来验证设备的发布/订阅行为是否正确。

MQTTX 是目前最流行的 MQTT 客户端, 但它基于 Electron, 资源占用大, 且无法与串口调试、波形显示联动。EmbedDebug 集成 MQTT 客户端后, 开发者可以在同一个工具里: 通过串口查看设备日志 + 通过 MQTT 验证设备上报的数据 + 波形对比串口原始数据和 MQTT 上报数据的一致性。这种多通道联动调试体验是其他工具做不到的。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F14-R1 | MQTT 5.0 连接: 支持 TCP/WebSocket 传输, TLS 加密, 用户名/密码认证 | P0 |
| F14-R2 | 订阅管理: 主题订阅/取消订阅, 通配符(+/#)支持, QoS 0/1/2 选择 | P0 |
| F14-R3 | 消息发布: 指定主题/QoS/retain 发布消息, 支持 HEX 和文本 payload | P0 |
| F14-R4 | 主题树浏览器: 可视化展示已订阅主题的层级结构 | P1 |
| F14-R5 | 消息流视图: 按时间序列显示所有收发的消息(类似终端) | P0 |
| F14-R6 | Retained 消息查看: 获取并显示指定主题的 retained 消息 | P1 |
| F14-R7 | Last Will 配置: 设置遗嘱主题和消息 | P2 |
| F14-R8 | MQTT 消息流入终端/波形: payload 数据可通过协议引擎解析并接入波形 | P1 |
| F14-R9 | 连接状态监控: 连接/断开/重连状态, Keep-Alive 超时提示 | P1 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `MqttConnection` | `connection/mqtt/MqttConnection.h/cpp` | 基础设施层 | IConnection 实现, MQTT 通信 |
| `MqttClientBackend` | `connection/mqtt/MqttClientBackend.h/cpp` | 基础设施层 | Qt MQTT 或 Mosquitto 库封装 |
| `MqttTopicModel` | `connection/mqtt/MqttTopicModel.h/cpp` | 数据层 | 主题树形数据模型 |
| `MqttMessageQueue` | `connection/mqtt/MqttMessageQueue.h/cpp` | 数据层 | 消息缓冲和 QoS 管理 |
| `MqttConfigPanel` | `connection/mqtt/MqttConfigPanel.h/cpp` | 表现层 | 连接配置(Broker/端口/认证/TLS) |
| `MqttTopicTreeWidget` | `connection/mqtt/MqttTopicTreeWidget.h/cpp` | 表现层 | 主题树浏览器 |
| `MqttMessageView` | `connection/mqtt/MqttMessageView.h/cpp` | 表现层 | 消息流列表视图 |
| `MqttPublishWidget` | `connection/mqtt/MqttPublishWidget.h/cpp` | 表现层 | 消息发布面板 |

### 设计要点

- MQTT 库选择: 优先 `QtMqtt` 模块(如果 Qt 6.8.3 可用), 备选 Mosquitto 嵌入式客户端
- `MqttConnection` 实现 `IConnection`, `write()` 对应 `publish()`, `dataReceived` 对应订阅回调
- 主题树使用 `QTreeWidget`, 按 "/" 分割构建层级节点, 支持通配符高亮
- 消息流视图复用 `TerminalModel` 的数据管道, MQTT payload 作为 QByteArray 流入
- TLS 支持: 如果编译了 Qt SSL 模块, 则支持 mqtts:// 和 wss://
- Keep-Alive 通过 `QTimer` 实现, 断线自动重连

### 验收标准

1. 可连接 MQTT Broker 并完成订阅/发布操作
2. 收到的消息实时显示在消息流视图
3. 主题树正确展示层级结构, 通配符订阅显示所有匹配主题
4. MQTT payload 数据可流入终端和波形通道
5. Broker 不可达时显示错误提示, 不崩溃
6. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F15: TCP/UDP 增强

> Priority: P1 | Module: `connection/tcp/` | Estimate: ~1000行 | Dependencies: 无

### 动机

当前 TCP/UDP 连接只支持单连接客户端模式。实际调试场景中:
- 需要同时维护多个 TCP 连接(比如连接设备的控制端口和数据端口)
- 需要监听模式(设备主动连接 EmbedDebug 作为 TCP Server)
- UDP 组播在音视频嵌入式设备调试中很常见
- TLS 加密连接在安全相关设备调试中不可或缺

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F15-R1 | 多连接管理器: 同时管理多个独立的 TCP/UDP 连接, 标签页切换 | P0 |
| F15-R2 | TCP Server 模式: 监听指定端口, 接受多个客户端连接 | P0 |
| F15-R3 | UDP 组播: 加入/离开组播组, 发送组播数据 | P1 |
| F15-R4 | SSL/TLS: TCP 连接支持 TLS 加密, 证书验证, 自签名证书 | P1 |
| F15-R5 | 连接管理面板: 所有活跃连接的状态/统计/快速切换 | P1 |
| F15-R6 | UDP 广播: 支持广播地址发送和接收 | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `MultiConnectionManager` | `connection/tcp/MultiConnectionManager.h/cpp` | 业务层 | 多连接生命周期管理 |
| `TcpServerConnection` | `connection/tcp/TcpServerConnection.h/cpp` | 基础设施层 | TCP Server 监听和客户端管理 |
| `UdpMulticastConnection` | `connection/tcp/UdpMulticastConnection.h/cpp` | 基础设施层 | UDP 组播/广播连接 |
| `TlsConnection` | `connection/tcp/TlsConnection.h/cpp` | 基础设施层 | TLS 加密 TCP 连接(QSslSocket) |
| `ConnectionManagerWidget` | `connection/tcp/ConnectionManagerWidget.h/cpp` | 表现层 | 多连接状态面板 |

### 设计要点

- `TcpServerConnection` 继承 `IConnection`, 内部维护 `QTcpServer` + 多个 `QTcpSocket`
- 每个连接的客户端作为独立数据源, 通过 `dataReceived` 信号携带客户端标识
- `MultiConnectionManager` 不替代 `ConnectionController`, 而是管理多个 `ConnectionController` 实例
- TLS 连接复用现有 `TcpConnection`, 通过 `configure()` 参数 `"ssl": true` 切换
- SSL 证书管理: 支持系统证书库 + 自定义 CA + 忽略验证(调试用)

### 验收标准

1. 可同时建立 2 个以上 TCP 连接并独立收发数据
2. TCP Server 模式可接受外部客户端连接
3. UDP 组播数据可正确收发
4. TLS 连接可建立加密通信, 自签名证书可配置
5. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F16: SPI/I2C 桥接

> Priority: P1 | Module: `connection/spi_i2c/` | Estimate: ~1400行 | Dependencies: 无

### 动机

SPI 和 I2C 是嵌入式系统中两类最常用的芯片级总线。传感器(IMU/温湿度/压力)、存储器(EEPROM/Flash)、显示屏(OLED/LCD)、无线模块(NRF24/LoRa) 都通过 SPI 或 I2C 与主控通信。

开发过程中经常需要独立于 MCU 直接与这些外设通信: 读取传感器原始数据验证寄存器配置、烧录 SPI Flash、扫描 I2C 总线上的设备地址。目前这类操作需要专门的适配器(如 Aardvark, Bus Pirate, FTDI FT232H), 每个适配器都自带一套独立的上位机软件。

EmbedDebug 通过 USB-to-SPI/I2C 桥接适配器支持, 让这些操作统一在一个工具里完成。配合协议解析引擎和波形显示, 开发者可以直观看到寄存器值的变化趋势。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F16-R1 | FTDI FT232H/FT2232 支持: 通过 D2XX 驱动实现 SPI/I2C 通信 | P0 |
| F16-R2 | CH347 支持: CH347T 的 SPI/I2C 模式, 通过厂商 DLL | P0 |
| F16-R3 | SPI 通信: 配置时钟频率/CPOL/CPHA/CS 模式, 全双工读写 | P0 |
| F16-R4 | I2C 通信: 7位/10位地址, 读/写/重复起始, 支持 I2C 扫描 | P0 |
| F16-R5 | I2C 设备扫描: 自动扫描 0x00~0x7F 地址, 显示响应的设备 | P0 |
| F16-R6 | 寄存器读写器: 指定寄存器地址读/写, 支持 8/16/32 位地址 | P1 |
| F16-R7 | 寄存器批量读取: 连续读取寄存器范围, 表格/波形显示 | P1 |
| F16-R8 | SPI Flash 操作: 读取 JEDEC ID, 扇区擦除, 编程, 整片读取 | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `SpiConnection` | `connection/spi_i2c/SpiConnection.h/cpp` | 基础设施层 | IConnection 实现, SPI 总线通信 |
| `I2cConnection` | `connection/spi_i2c/I2cConnection.h/cpp` | 基础设施层 | IConnection 实现, I2C 总线通信 |
| `FtdiBridge` | `connection/spi_i2c/FtdiBridge.h/cpp` | 基础设施层 | FTDI D2XX 驱动封装(MPSSE) |
| `Ch347Bridge` | `connection/spi_i2c/Ch347Bridge.h/cpp` | 基础设施层 | CH347 DLL 驱动封装 |
| `I2cScanner` | `connection/spi_i2c/I2cScanner.h/cpp` | 数据层 | I2C 总线地址扫描 |
| `RegisterEditor` | `connection/spi_i2c/RegisterEditor.h/cpp` | 表现层 | 寄存器地址/值编辑器 |
| `SpiI2cConfigPanel` | `connection/spi_i2c/SpiI2cConfigPanel.h/cpp` | 表现层 | SPI/I2C 参数配置 |
| `I2cScanWidget` | `connection/spi_i2c/I2cScanWidget.h/cpp` | 表现层 | I2C 扫描结果网格视图 |

### 设计要点

- SPI/I2C 各自实现 `IConnection`, 通过 `configure()` 的 `"mode": "spi"` 或 `"mode": "i2c"` 区分
- FTDI 桥接通过 `QLibrary` 动态加载 `ftd2xx.dll`, MPSSE 命令控制 SPI/I2C
- CH347 桥接通过 `QLibrary` 加载 `CH347DLL.DLL`
- `I2cScanner` 依次对每个地址发送 ACK 检测, 超时未响应视为无设备
- 寄存器读写器支持多字节地址和不同位宽(8/16/32)
- 扫描结果使用 `QTableWidget` 显示, 7×16 网格(行=高4位, 列=低4位)

### 验收标准

1. FTDI FT232H 可完成 SPI 发送和接收
2. I2C 设备扫描可发现总线上的设备
3. 寄存器读写器可读写指定寄存器
4. 适配器 DLL 缺失时显示提示, 不崩溃
5. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F17: WebSocket 客户端

> Priority: P1 | Module: `connection/ws/` | Estimate: ~800行 | Dependencies: 无

### 动机

WebSocket 正在成为嵌入式设备的远程管理接口。ESP32/树莓派等设备通过 WebSocket 实现实时数据推送和远程控制。调试这类设备需要一个 WebSocket 客户端来验证通信协议。

Qt 自带 `QWebSocket` 类, 实现成本很低。作为 `IConnection` 的新实现, WebSocket 数据可以直接接入终端和协议解析管道。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F17-R1 | WebSocket 连接: 支持 ws:// 和 wss:// (TLS), 自定义 HTTP Header | P0 |
| F17-R2 | 双向消息收发: 文本帧和二进制帧, 自动检测帧类型 | P0 |
| F17-R3 | 消息历史: 按时间序列显示收发消息, 区分方向 | P0 |
| F17-R4 | Ping/Pong: 心跳检测和延迟测量 | P1 |
| F17-R5 | 连接参数: 超时/子协议/Origin 头设置 | P1 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `WebSocketConnection` | `connection/ws/WebSocketConnection.h/cpp` | 基础设施层 | IConnection 实现, WebSocket 通信 |
| `WsConfigPanel` | `connection/ws/WsConfigPanel.h/cpp` | 表现层 | WebSocket 连接配置面板 |
| `WsMessageLog` | `connection/ws/WsMessageLog.h/cpp` | 数据层 | 消息历史记录和检索 |

### 设计要点

- 基于 `QWebSocket`, 几乎直接映射到 `IConnection` 接口
- `write()` 根据数据内容自动判断发送文本帧还是二进制帧
- WSS 支持 `QSslConfiguration`, 可配置 CA 证书和忽略验证
- 消息日志复用 `TerminalModel` 的数据结构

### 验收标准

1. 可连接 WebSocket 服务器并发送/接收消息
2. WSS 加密连接正常工作
3. 消息在终端实时显示
4. 连接断开后自动提示, 不崩溃
5. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F18: Modbus 主站/从站

> Priority: P2 | Module: `protocol/modbus/` | Estimate: ~1800行 | Dependencies: F2(协议引擎)

### 动机

Modbus 是工业自动化领域的元老级协议, 至今仍是 PLC、变频器、仪表、传感器最常用的通信协议。Modbus RTU(串口)和 Modbus TCP(以太网)几乎覆盖了所有工业设备的调试场景。

EmbedDebug 已经支持串口和 TCP 连接, 缺少的是 Modbus 应用层的协议实现。当前虽然有自定义协议引擎(F2), 但 Modbus 的功能码、寄存器操作、CRC 校验、异常码处理等细节完全手配太麻烦。需要一个专门的 Modbus 主站/从站实现, 提供开箱即用的寄存器读写、线圈操作、设备扫描等功能。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F18-R1 | Modbus RTU 模式: 串口传输, CRC16 校验, 波特率/校验位配置 | P0 |
| F18-R2 | Modbus ASCII 模式: ASCII 传输, LRC 校验 | P1 |
| F18-R3 | Modbus TCP 模式: TCP 传输, MBAP 头, 端口502 | P0 |
| F18-R4 | 主站操作: 读保持寄存器(FC03)/读输入寄存器(FC04)/写单个寄存器(FC06)/写多个寄存器(FC16) | P0 |
| F18-R5 | 线圈操作: 读线圈(FC01)/读离散输入(FC02)/写单个线圈(FC05)/写多个线圈(FC15) | P1 |
| F18-R6 | 设备扫描: 扫描 1~247 从站地址, 识别响应设备 | P0 |
| F18-R7 | 寄存器监视器: 周期性轮询寄存器, 表格/波形实时显示 | P1 |
| F18-R8 | 从站模拟: 模拟 Modbus 从站响应, 用于测试主站设备 | P2 |
| F18-R9 | 异常码处理: 解析并显示 Modbus 异常响应(ILLEGAL_FUNCTION/DATA_ADDRESS等) | P0 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `ModbusMaster` | `protocol/modbus/ModbusMaster.h/cpp` | 业务层 | Modbus 主站操作封装 |
| `ModbusSlave` | `protocol/modbus/ModbusSlave.h/cpp` | 业务层 | Modbus 从站模拟 |
| `ModbusFrameCodec` | `protocol/modbus/ModbusFrameCodec.h/cpp` | 数据层 | ADU/PDU 编解码, CRC/LRC 计算 |
| `ModbusDataModel` | `protocol/modbus/ModbusDataModel.h/cpp` | 数据层 | 寄存器/线圈数据模型 |
| `ModbusScanner` | `protocol/modbus/ModbusScanner.h/cpp` | 业务层 | 从站地址扫描 |
| `ModbusRegisterMonitor` | `protocol/modbus/ModbusRegisterMonitor.h/cpp` | 业务层 | 寄存器轮询和变化监控 |
| `ModbusMasterPanel` | `protocol/modbus/ModbusMasterPanel.h/cpp` | 表现层 | 主站操作面板 |
| `ModbusRegisterView` | `protocol/modbus/ModbusRegisterView.h/cpp` | 表现层 | 寄存器表格视图 |
| `ModbusConfigPanel` | `protocol/modbus/ModbusConfigPanel.h/cpp` | 表现层 | Modbus 模式/参数配置 |

### 设计要点

- Modbus 不实现 `IConnection`, 而是在已有的串口/TCP 连接之上运行
- `ModbusMaster` 持有一个 `IConnection*` 指针, 通过它发送原始帧
- RTU 模式的 CRC16 校验使用公共组件 `CrcCalculator` (已有)
- 寄存器视图使用 `QTableWidget`, 行=寄存器地址, 列=当前值/上次值/变化标志
- 轮询通过 `QTimer` 定时触发, 间隔可配置
- 从站模拟器在本地维护 `ModbusDataModel`, 响应主站的读写请求

### 验收标准

1. Modbus RTU 模式可读写远程从站寄存器
2. Modbus TCP 模式可连接远程 Modbus 设备
3. 设备扫描可发现总线上活跃的从站
4. 寄存器监视器周期刷新, 值变化时高亮
5. Modbus 异常响应正确解析和显示
6. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F19: Protobuf/FlatBuffers 解码

> Priority: P2 | Module: `protocol/protobuf/` | Estimate: ~1200行 | Dependencies: F2(协议引擎)

### 动机

现代嵌入式设备越来越多地使用 Protocol Buffers 或 FlatBuffers 作为数据序列化格式。相比 JSON, 它们更紧凑、解析更快、跨语言兼容。特别是在机器人(ROS2)、无人机(MAVLink protobuf)、物联网平台(gRPC) 场景中。

调试这类设备时, 收到的二进制数据无法直接阅读, 必须有对应的 `.proto` 或 `.fbs` 文件才能解码。EmbedDebug 导入 schema 文件后, 可以将二进制 payload 自动解码为结构化数据, 配合波形引擎显示关键字段的变化趋势。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F19-R1 | .proto 文件导入: 解析 proto2/proto3 语法定义, 提取 message/enum/field | P0 |
| F19-R2 | Protobuf 二进制解码: 按导入的 schema 解码二进制 payload, 显示字段名/类型/值 | P0 |
| F19-R3 | .fbs 文件导入: 解析 FlatBuffers schema 定义 | P1 |
| F19-R4 | FlatBuffers 二进制解码: 解码 FlatBuffers 二进制数据 | P1 |
| F19-R5 | 字段映射: 解码后的数值字段可映射到波形通道 | P0 |
| F19-R6 | 嵌套消息展开: 嵌套的 message 字段可展开/折叠显示 | P1 |
| F19-R7 | JSON 输出: 解码结果可导出为 JSON 格式 | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `ProtobufSchema` | `protocol/protobuf/ProtobufSchema.h/cpp` | 数据层 | .proto 文件解析和消息定义模型 |
| `ProtobufDecoder` | `protocol/protobuf/ProtobufDecoder.h/cpp` | 业务层 | 二进制 Protobuf 数据解码 |
| `FlatBuffersSchema` | `protocol/protobuf/FlatBuffersSchema.h/cpp` | 数据层 | .fbs 文件解析和类型定义模型 |
| `FlatBuffersDecoder` | `protocol/protobuf/FlatBuffersDecoder.h/cpp` | 业务层 | 二进制 FlatBuffers 数据解码 |
| `SchemaFieldMapper` | `protocol/protobuf/SchemaFieldMapper.h/cpp` | 数据层 | 解码字段→波形通道映射 |
| `ProtobufSchemaWidget` | `protocol/protobuf/ProtobufSchemaWidget.h/cpp` | 表现层 | Schema 浏览和字段选择 |
| `DecodedMessageView` | `protocol/protobuf/DecodedMessageView.h/cpp` | 表现层 | 解码后的消息树形显示 |

### 设计要点

- .proto 解析器实现最简版: 仅解析 `message`, `enum`, `scalar types`, `repeated`, `nested message`
- 不依赖 `protoc` 编译器, 直接从 .proto 文本解析
- Protobuf 解码器手写 varint/zigzag/length-delimited 解析, 不依赖 libprotobuf
- FlatBuffers 解码参考官方的 FlatBuffers C API, 通过偏移量直接读取
- 解码结果以 `QVariantMap` 形式传递给 `ProtocolFieldMapper`
- JSON 导出使用 `QJsonDocument::fromJson()`

### 验收标准

1. .proto 文件可导入并显示消息定义结构
2. 二进制数据可按 schema 正确解码, 字段名/值正确
3. 解码的数值字段可绑定到波形通道实时显示
4. 嵌套消息可展开查看
5. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F20: USB 调试

> Priority: P2 | Module: `connection/usb/` | Estimate: ~1500行 | Dependencies: 无

### 动机

USB 是嵌入式设备与 PC 连接最通用的接口。调试 USB 设备固件时, 需要查看设备描述符、监控端点数据传输、测试批量/中断传输。Wireshark 的 USB 抓包功能强大但过于重量级, USBlyser 是收费软件。

EmbedDebug 集成 libusb 后, 可以直接与 USB 设备通信(绕过内核驱动), 这对开发自定义 USB 设备(如 HID 设备、自定义 USB 类)的固件工程师来说非常有价值。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F20-R1 | 设备枚举: 列出所有 USB 设备, 显示 VID/PID/厂商/产品名/序列号 | P0 |
| F20-R2 | 描述符查看器: 解析并显示 Device/Configuration/Interface/Endpoint 描述符 | P0 |
| F20-R3 | 端点监控: 选择接口和端点, 实时监控 IN/OUT 传输数据 | P0 |
| F20-R4 | 批量传输: 批量(Bulk)端点的读/写操作 | P0 |
| F20-R5 | 中断传输: 中断(Interrupt)端点的读/写操作 | P1 |
| F20-R6 | 设备固件升级: 配合 DFU 模式进行 USB 固件下载 | P2 |
| F20-R7 | USB 抓包: 基于 WinUSB/libusb 捕获 USB 传输数据 | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `UsbConnection` | `connection/usb/UsbConnection.h/cpp` | 基础设施层 | IConnection 实现, USB 端点通信 |
| `UsbDeviceList` | `connection/usb/UsbDeviceList.h/cpp` | 数据层 | USB 设备枚举和缓存 |
| `UsbDescriptorParser` | `connection/usb/UsbDescriptorParser.h/cpp` | 数据层 | USB 描述符解析和格式化 |
| `UsbTransferEngine` | `connection/usb/UsbTransferEngine.h/cpp` | 基础设施层 | Bulk/Interrupt 传输引擎 |
| `UsbDeviceTreeWidget` | `connection/usb/UsbDeviceTreeWidget.h/cpp` | 表现层 | USB 设备树形列表 |
| `UsbDescriptorView` | `connection/usb/UsbDescriptorView.h/cpp` | 表现层 | 描述符详细视图 |
| `UsbConfigPanel` | `connection/usb/UsbConfigPanel.h/cpp` | 表现层 | USB 设备和端点选择 |

### 设计要点

- libusb 通过 vcpkg 或直接引入源码编译, Windows 需要 WinUSB 驱动
- `UsbConnection` 实现 `IConnection`, `write()` 对应 Bulk/Interrupt OUT, `dataReceived` 对应 IN
- 设备枚举使用 `libusb_get_device_list()`, 缓存避免重复扫描
- 描述符解析按照 USB 2.0/3.0 规范, 支持标准描述符 + HID 描述符
- WinUSB 驱动安装提示: 检测到设备但无法打开时, 引导用户安装 WinUSB

### 验收标准

1. 可枚举并列出所有 USB 设备
2. 描述符解析正确, 字段名称和值可读
3. 可打开设备并在 Bulk/Interrupt 端点进行读写
4. 端点数据在终端实时显示
5. libusb 不可用时显示提示, 不崩溃
6. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F21: 正则终端过滤器

> Priority: P3 | Module: `terminal/filter/` | Estimate: ~600行 | Dependencies: 无

### 动机

嵌入式调试中, 终端输出常常被大量调试日志淹没, 开发者只关心特定的信息。比如只想看"ERROR"开头的行, 或者过滤出包含特定寄存器地址的日志。传统的终端搜索是"跳转到下一个匹配", 而不是"实时过滤只显示匹配的行"。

实时正则过滤 + 颜色高亮 + 捕获组提取, 让开发者在海量日志中瞬间定位关键信息。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F21-R1 | 正则表达式过滤: 输入正则, 终端实时只显示匹配行(或排除匹配行) | P0 |
| F21-R2 | 颜色高亮: 匹配的文本用指定颜色高亮, 支持多组规则不同颜色 | P0 |
| F21-R3 | 捕获组提取: 从匹配结果中提取捕获组, 以表格形式显示 | P1 |
| F21-R4 | 过滤规则管理: 多条过滤规则的添加/编辑/删除/启用/禁用 | P1 |
| F21-R5 | 过滤规则保存: 规则保存到会话, 下次打开自动加载 | P2 |
| F21-R6 | 统计: 匹配行数/总行数/匹配率 | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `TerminalFilter` | `terminal/filter/TerminalFilter.h/cpp` | 数据层 | 正则匹配和过滤逻辑 |
| `FilterRule` | `terminal/filter/FilterRule.h/cpp` | 数据层 | 过滤规则数据模型 |
| `CaptureGroupExtractor` | `terminal/filter/CaptureGroupExtractor.h/cpp` | 数据层 | 捕获组提取和格式化 |
| `FilterRuleWidget` | `terminal/filter/FilterRuleWidget.h/cpp` | 表现层 | 过滤规则编辑和列表 |
| `FilterResultWidget` | `terminal/filter/FilterResultWidget.h/cpp` | 表现层 | 过滤结果和捕获组表格 |

### 设计要点

- `TerminalFilter` 连接到 `TerminalModel` 的 `dataAdded` 信号, 实时过滤
- 正则使用 `QRegularExpression`, 支持所有标准正则语法
- 颜色高亮通过 `QTextCharFormat` 实现, 在 `TerminalWidget` 的渲染层处理
- 捕获组提取使用 `QRegularExpressionMatch::captured()`, 结果存入表格模型
- 过滤逻辑在数据层完成, 避免大量文本在 UI 线程中匹配

### 验收标准

1. 输入正则后终端实时显示过滤结果
2. 匹配文本颜色高亮, 多条规则不同颜色
3. 捕获组正确提取并显示在表格
4. 正则语法错误时显示提示, 不崩溃
5. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F22: 校验和计算器

> Priority: P3 | Module: `utils/checksum/` | Estimate: ~500行 | Dependencies: 无

### 动机

嵌入式开发中, 校验和计算是高频操作。配置一个协议帧、验证一段 Flash 数据、调试一个 CRC 校验失败的通信问题, 都需要快速计算各种校验和。目前开发者要么用在线工具(需要网络), 要么手写 Python 脚本(效率低)。

EmbedDebug 已经有 `CrcCalculator` 公共组件, 只需要一个 UI 包装器暴露给用户使用。支持自定义多项式的 CRC 计算, 可以覆盖几乎所有嵌入式场景。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F22-R1 | 校验算法: CRC8/CRC16/CRC32/XOR8/Sum8/Sum16/Sum32 | P0 |
| F22-R2 | CRC 标准预设: CRC16-CCITT/CRC16-Modbus/CRC32/MPEG2 等 | P0 |
| F22-R3 | 自定义 CRC: 可指定多项式/初始值/异或输出/输入输出反转 | P0 |
| F22-R4 | 输入方式: HEX 输入/文本输入/从终端拖拽数据 | P1 |
| F22-R5 | 结果复制: 一键复制计算结果到剪贴板 | P1 |
| F22-R6 | 批量计算: 选择文件计算整体校验和 | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `ChecksumCalculator` | `utils/checksum/ChecksumCalculator.h/cpp` | 数据层 | 复用已有 CrcCalculator, 扩展 XOR/Sum 算法 |
| `ChecksumWidget` | `utils/checksum/ChecksumWidget.h/cpp` | 表现层 | 校验和计算器 UI 面板 |
| `CrcPresetLibrary` | `utils/checksum/CrcPresetLibrary.h/cpp` | 数据层 | 预设 CRC 参数集合 |

### 设计要点

- `ChecksumCalculator` 包装已有的 `CrcCalculator`, 新增 XOR/Sum 算法
- 预设库是 JSON 配置, 包含常用 CRC 变种的参数
- UI 使用停靠面板(QDockWidget), 可拖拽到任意位置
- 输入区域支持 HEX(带空格) 和 ASCII 双模式

### 验收标准

1. CRC16-Modbus 和 CRC32 计算结果与在线工具一致
2. 自定义多项式计算正确
3. XOR/Sum 计算正确
4. 结果一键复制到剪贴板
5. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F23: ASCII/表格转换器

> Priority: P3 | Module: `utils/converter/` | Estimate: ~400行 | Dependencies: 无

### 动机

嵌入式开发者的日常: Hex 转 ASCII, Base64 解码, URL 编码转换, 字节序翻转。每次都开浏览器查在线工具太麻烦。把这些常用转换集成到 EmbedDebug 里, 作为一个停靠面板随时可用。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F23-R1 | Hex ↔ ASCII 互转: 输入 HEX 输出 ASCII, 反之亦然 | P0 |
| F23-R2 | Base64 编码/解码: 文本和二进制的 Base64 互转 | P0 |
| F23-R3 | URL 编码/解码: RFC 3986 标准 URL 编码 | P1 |
| F23-R4 | 字节序翻转: 大端↔小端, 16/32/64 位 | P1 |
| F23-R5 | 数值进制转换: HEX/DEC/OCT/BIN 互转, 支持 8/16/32/64 位 | P0 |
| F23-R6 | 双向实时转换: 输入任意一侧, 另一侧自动更新 | P0 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `DataConverter` | `utils/converter/DataConverter.h/cpp` | 数据层 | 各种编码转换逻辑 |
| `ConverterWidget` | `utils/converter/ConverterWidget.h/cpp` | 表现层 | 双栏转换器 UI |

### 设计要点

- `DataConverter` 是纯函数工具类, 所有方法为 static
- 双栏 UI: 左侧输入区, 右侧输出区, 中间下拉选择转换类型
- 使用 `QTextCodec` 处理各种文本编码(UTF-8/GBK/Latin-1)
- Base64 使用 `QByteArray::toBase64()` / `fromBase64()`
- 进制转换使用 `QString::number(value, base)`

### 验收标准

1. Hex/ASCII 互转正确, 含不可打印字符处理
2. Base64 编解码与标准工具结果一致
3. 字节序翻转正确处理 16/32/64 位
4. 输入侧修改后输出侧实时更新
5. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F24: 时间戳分析器

> Priority: P3 | Module: `utils/timestamp/` | Estimate: ~300行 | Dependencies: 无

### 动机

嵌入式设备经常输出 Unix 时间戳或相对时间戳。调试时需要把这些数字转换成人类可读的日期时间, 或者反过来生成时间戳用于测试。一个轻量的内嵌工具省去每次搜索在线转换器。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F24-R1 | Unix 时间戳转换: 秒级/毫秒级/微秒级时间戳 ↔ 日期时间字符串 | P0 |
| F24-R2 | 当前时间戳: 显示当前 Unix 时间戳, 实时刷新 | P0 |
| F24-R3 | 时间差计算: 两个时间戳之间的差值(天/时/分/秒/毫秒) | P1 |
| F24-R4 | 时间格式选择: 支持多种输出格式(ISO 8601/自定义格式) | P1 |
| F24-R5 | 批量转换: 粘贴多行时间戳批量转换 | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `TimestampConverter` | `utils/timestamp/TimestampConverter.h/cpp` | 数据层 | 时间戳与日期时间的互转 |
| `TimestampWidget` | `utils/timestamp/TimestampWidget.h/cpp` | 表现层 | 时间戳转换器 UI 面板 |

### 设计要点

- 使用 `QDateTime::fromSecsSinceEpoch()` 和 `toSecsSinceEpoch()`
- 自动检测时间戳精度: <1e10 视为秒, <1e13 视为毫秒, <1e16 视为微秒
- 时区支持: 本地时间和 UTC 时间切换
- 批量转换使用 `QTextEdit` 多行输入

### 验收标准

1. Unix 时间戳正确转换为可读日期时间
2. 秒/毫秒/微秒精度自动识别
3. 时间差计算正确
4. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F25: 数据包构建器

> Priority: P3 | Module: `utils/packet/` | Estimate: ~700行 | Dependencies: F22(校验和)

### 动机

调试通信协议时, 经常需要手动构造特定格式的数据包发送给设备。比如构造一个 Modbus 请求帧、一个自定义 TLV 命令、一个传感器配置包。目前需要手动计算偏移量和校验和, 容易出错。

可视化数据包构建器让用户通过表单界面定义每个字段(名称/类型/长度/值), 自动计算校验和, 生成完整的二进制数据包, 一键发送到当前连接。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F25-R1 | 字段编辑器: 添加/删除/排序字段, 每个字段指定名称/类型(uint8~uint64)/字节序/值 | P0 |
| F25-R2 | 校验和自动计算: 选择校验和类型(CRC/XOR/Sum), 自动计算并插入到指定位置 | P0 |
| F25-R3 | 预览区: 实时显示构建结果(HEX + ASCII) | P0 |
| F25-R4 | 长度字段自动计算: 指定某个字段为长度字段, 自动填入后续字段总长度 | P1 |
| F25-R5 | 数据包模板: 保存常用数据包结构为模板, 快速加载 | P1 |
| F25-R6 | 一键发送: 将构建的数据包直接发送到当前连接 | P0 |
| F25-R7 | 数据包导入: 从 HEX 文本导入已有数据包, 反向解析字段 | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `PacketField` | `utils/packet/PacketField.h/cpp` | 数据层 | 单个字段的数据模型 |
| `PacketBuilder` | `utils/packet/PacketBuilder.h/cpp` | 业务层 | 多字段组合和校验和计算 |
| `PacketTemplate` | `utils/packet/PacketTemplate.h/cpp` | 数据层 | 数据包模板的保存和加载 |
| `PacketBuilderWidget` | `utils/packet/PacketBuilderWidget.h/cpp` | 表现层 | 数据包构建器 UI |
| `PacketFieldEditor` | `utils/packet/PacketFieldEditor.h/cpp` | 表现层 | 单个字段的编辑控件 |

### 设计要点

- `PacketField` 包含: name, type(uint8/uint16/uint32/uint64/bytes), endianness, value
- `PacketBuilder` 维护 `QList<PacketField>`, 提供 `toByteArray()` 和校验和插入
- 校验和计算复用 `ChecksumCalculator`
- 长度字段: 标记某个字段为 "length_of: [字段范围]", 自动计算
- 预览区使用 `QTextEdit`, 上方 HEX 下方 ASCII, 字段边界用竖线分隔
- 模板保存为 JSON 格式

### 验收标准

1. 可添加多个字段并设置类型/字节序/值
2. 预览区实时显示 HEX 和 ASCII 结果
3. 校验和自动计算并插入到正确位置
4. 一键发送数据包到当前连接
5. 模板可保存和加载
6. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## F26: 设备注册表

> Priority: P3 | Module: `core/device/` | Estimate: ~600行 | Dependencies: 无

### 动机

开发者在日常工作中经常在多个嵌入式设备之间切换: STM32 开发板 + ESP32 模组 + RK3568 核心板 + 各种传感器模块。每个设备有不同的连接参数(波特率/端口/TCP地址)、协议配置(JustFloat/Modbus/自定义)、常用指令集(AT命令/寄存器读命令)。

目前 EmbedDebug 的会话管理(F8)保存了窗口状态, 但缺少"设备"维度的组织。需要一个设备注册表, 让开发者保存每个设备的完整调试配置, 一键切换。

### 需求

| ID | Description | Priority |
|----|-------------|----------|
| F26-R1 | 设备档案: 设备名称/型号/描述/标签 + 连接配置 + 协议配置 + 快捷命令列表 | P0 |
| F26-R2 | 设备列表: 按标签/名称筛选, 缩略图/列表视图切换 | P0 |
| F26-R3 | 一键连接: 选择设备后自动应用所有配置并建立连接 | P0 |
| F26-R4 | 快捷命令库: 每个设备维护一组常用命令(AT/HEX/文本), 支持分组 | P1 |
| F26-R5 | 设备导入/导出: JSON 文件, 可在团队间分享设备配置 | P1 |
| F26-R6 | 设备图标: 可为设备设置图标(内置图标集或自定义图片) | P2 |

### 新增类

| Class | File | Layer | Responsibility |
|-------|------|-------|---------------|
| `DeviceProfile` | `core/device/DeviceProfile.h/cpp` | 数据层 | 设备档案数据模型 |
| `DeviceRegistry` | `core/device/DeviceRegistry.h/cpp` | 数据层 | 设备注册表的 CRUD 和持久化 |
| `QuickCommandLibrary` | `core/device/QuickCommandLibrary.h/cpp` | 数据层 | 快捷命令库的数据管理 |
| `DeviceListWidget` | `core/device/DeviceListWidget.h/cpp` | 表现层 | 设备列表 UI |
| `DeviceProfileEditor` | `core/device/DeviceProfileEditor.h/cpp` | 表现层 | 设备档案编辑对话框 |

### 设计要点

- `DeviceProfile` 聚合了连接配置(QVariantMap) + 协议配置 + 快捷命令列表
- `DeviceRegistry` 使用 JSON 文件存储: `settings/devices/{uuid}.json`
- 一键连接: `DeviceRegistry::applyProfile()` 调用各 Manager 的配置加载方法
- 快捷命令复用现有的 `SendController` 发送机制
- 设备列表使用 `QListView` + 自定义 delegate
- 与工程会话(F8)的关系: 工程包含多个设备, 设备可在工程间共享

### 验收标准

1. 可创建/编辑/删除设备档案
2. 一键连接后连接参数和协议配置正确应用
3. 快捷命令可添加到设备并发送
4. 设备配置可导出为 JSON 并在其他电脑导入
5. 所有新增文件 .cpp ≤ 500行, .h ≤ 200行

---

## 依赖关系图

```
┌─────────────────────────────────────────────────────────────┐
│                        连接层                                │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │F12 BLE   │  │F13 CAN   │  │F14 MQTT  │  │F15 TCP+  │   │
│  │    ↕     │  │          │  │    ↕     │  │          │   │
│  │F17 WS    │  │          │  │F17 WS    │  │  SSL/TLS │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐                 │
│  │F16 SPI   │  │F20 USB   │  │ F6 RTT   │  (已规划)       │
│  │  /I2C    │  │          │  │          │                  │
│  └──────────┘  └──────────┘  └──────────┘                 │
│          │            │                                     │
│          └──────┬─────┘                                     │
│                 │ 均实现 IConnection                         │
└─────────────────┼───────────────────────────────────────────┘
                  │
┌─────────────────┼───────────────────────────────────────────┐
│                 │        协议层                               │
│  ┌──────────────┴──────────┐                                │
│  │F2 自定义协议引擎(已规划)  │                                │
│  └───────┬──────┬──────┬───┘                                │
│          │      │      │                                    │
│  ┌───────┴──┐ ┌─┴────┐ ┌┴──────────┐                      │
│  │F18 Modbus│ │F19   │ │ 协议字段   │                      │
│  │          │ │Proto │ │ → 波形通道 │                      │
│  └──────────┘ │/Flat │ └───────────┘                      │
│               └──────┘                                     │
└────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                        工具层                                │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │F21 正则   │  │F22 校验   │  │F23 编码   │  │F24 时间戳│   │
│  │  过滤器   │  │  和计算   │  │  转换器   │  │  分析器  │   │
│  └──────────┘  └────┬─────┘  └──────────┘  └──────────┘   │
│                     │                                        │
│               ┌─────┴──────┐  ┌──────────┐                  │
│               │F25 数据包   │  │F26 设备   │                 │
│               │  构建器     │  │  注册表   │                 │
│               └────────────┘  └──────────┘                  │
└─────────────────────────────────────────────────────────────┘
```

### 依赖说明

| 依赖关系 | 原因 |
|---------|------|
| F14 → F17 | MQTT 可选 WebSocket 作为传输层 |
| F18 → F2 | Modbus 协议可作为协议引擎的内置模板 |
| F19 → F2 | Protobuf 解码结果需通过协议字段映射到波形 |
| F25 → F22 | 数据包构建器依赖校验和计算 |
| F12 ↔ F15 | BLE Secure 可能需要 TLS 能力 |
| F13 → 无 | CAN 总线独立于其他连接, 仅依赖 IConnection |
| F16 → 无 | SPI/I2C 桥接独立, 仅依赖 IConnection |
| F21,F22,F23,F24 → 无 | 工具类特性完全独立 |
| F26 → 无 | 设备注册表独立, 但可聚合所有其他特性的配置 |

---

## 里程碑规划

### Milestone 1: 总线级扩展 (MS1, 预计 4~6 周)

**目标**: EmbedDebug 从串口工具升级为总线级调试器, 覆盖 CAN 和 BLE 两大核心总线

| 顺序 | 特性 | 代码量 | 原因 |
|------|------|--------|------|
| 1 | **F13 CAN/CAN-FD 总线** | ~2400行 | 汽车电子/工业控制核心需求, DBC 解码是杀手级功能 |
| 2 | **F12 Bluetooth/BLE 调试** | ~2200行 | IoT/可穿戴设备核心需求, PC 端缺乏好用的集成工具 |
| | **小计** | **~4600行** | |

**交付标准**: EmbedDebug 可同时通过串口调试 MCU 固件 + 通过 CAN 分析总线数据 + 通过 BLE 监控无线设备

### Milestone 2: 通信层全覆盖 (MS2, 预计 6~8 周)

**目标**: 补齐所有主流通信方式, 实现"一个工具连接一切"

| 顺序 | 特性 | 代码量 | 原因 |
|------|------|--------|------|
| 3 | **F15 TCP/UDP 增强** | ~1000行 | 补齐多连接/Server/TLS 等基础能力 |
| 4 | **F17 WebSocket 客户端** | ~800行 | 实现成本低, IoT 场景常见 |
| 5 | **F14 MQTT 客户端** | ~1600行 | IoT 标准协议, 配合 WS 传输 |
| 6 | **F16 SPI/I2C 桥接** | ~1400行 | 芯片级调试, 差异化功能 |
| | **小计** | **~4800行** | |

**交付标准**: 开发者可通过 EmbedDebug 连接串口/TCP/UDP/WebSocket/MQTT/BLE/CAN/SPI/I2C 九种通信方式

### Milestone 3: 协议深度支持 (MS3, 预计 4~6 周)

**目标**: 深入主流工业/通信协议, 提供专业级协议分析能力

| 顺序 | 特性 | 代码量 | 原因 |
|------|------|--------|------|
| 7 | **F18 Modbus 主站/从站** | ~1800行 | 工业自动化标准协议 |
| 8 | **F19 Protobuf/FlatBuffers** | ~1200行 | 现代嵌入式数据序列化 |
| 9 | **F20 USB 调试** | ~1500行 | USB 固件开发利器 |
| | **小计** | **~4500行** | |

**交付标准**: Modbus 设备可读写寄存器, Protobuf 数据可解码显示, USB 设备可端点通信

### Milestone 4: 效率工具集 (MS4, 预计 3~4 周)

**目标**: 打磨日常调试效率, 提供完整的小工具生态

| 顺序 | 特性 | 代码量 | 原因 |
|------|------|--------|------|
| 10 | **F21 正则终端过滤器** | ~600行 | 高频使用, 实现成本低 |
| 11 | **F22 校验和计算器** | ~500行 | 复用已有 CrcCalculator |
| 12 | **F23 ASCII/表格转换器** | ~400行 | 高频使用, 实现成本极低 |
| 13 | **F24 时间戳分析器** | ~300行 | 实现成本最低 |
| 14 | **F25 数据包构建器** | ~700行 | 依赖 F22, 配合校验和自动计算 |
| 15 | **F26 设备注册表** | ~600行 | 多设备管理效率提升 |
| | **小计** | **~2600行** | |

**交付标准**: 嵌入式开发者日常需要的所有小工具都在 EmbedDebug 里

---

## 代码量估算

### 按里程碑汇总

| Milestone | 新增代码(行) | 新增文件数 | 新增模块数 | 预计周期 |
|-----------|-------------|-----------|-----------|---------|
| MS1: 总线扩展 | ~4,600 | ~32 | 2 (ble, can) | 4~6 周 |
| MS2: 通信全覆盖 | ~4,800 | ~34 | 4 (mqtt, tcp+, ws, spi_i2c) | 6~8 周 |
| MS3: 协议深度 | ~4,500 | ~30 | 3 (modbus, protobuf, usb) | 4~6 周 |
| MS4: 效率工具 | ~2,600 | ~22 | 5 (filter, checksum, converter, timestamp, packet, device) | 3~4 周 |
| **总计** | **~16,500** | **~118** | **14** | **17~24 周** |

### 按层级分布

| 层级 | 新增代码(行) | 占比 | 说明 |
|------|-------------|------|------|
| 基础设施层 | ~6,500 | 39% | IConnection 实现 + 平台适配 |
| 数据层 | ~4,000 | 24% | 数据模型 + 解析器 + 编解码 |
| 业务层 | ~2,500 | 15% | 协议引擎 + 扫描器 + 管理器 |
| 表现层 | ~3,500 | 22% | 配置面板 + 监视器 + 编辑器 |

### 与已有特性代码量对比

| 范围 | 特性数 | 代码行 | 文件数 |
|------|--------|--------|--------|
| F1~F11 (已有路线图) | 11 | ~8,600 | ~61 |
| F12~F26 (本次扩展) | 15 | ~16,500 | ~118 |
| **合计** | **26** | **~25,100** | **~179** |

---

## 技术约束与风险

### 外部依赖库

| 依赖 | 用途 | 许可证 | 风险 |
|------|------|--------|------|
| WinRT BLE API | BLE 通信 | Windows SDK | Windows 10+ 限定 |
| PCANBasic.dll | PCAN 适配器 | Proprietary | 需要安装 PCAN 驱动 |
| ftd2xx.dll | FTDI SPI/I2C | Proprietary | 需要安装 D2XX 驱动 |
| CH347DLL.DLL | CH347 适配器 | Proprietary | 需要安装 CH347 驱动 |
| libusb | USB 设备通信 | LGPL 2.1 | 静态链接需注意许可 |
| QtMqtt / Mosquitto | MQTT 通信 | GPLv3/EPL | 许可证兼容性 |

### 风险缓解

1. **平台适配器缺失**: 所有外部 DLL 通过 `QLibrary` 动态加载, 缺失时优雅降级, 显示安装提示
2. **BLE 限制**: Windows 10+ 才支持 WinRT BLE API, 旧系统显示"不支持"提示
3. **CAN 适配器成本**: SLCAN 适配器是最便宜的方案(几十元 CANable), 优先支持
4. **MQTT 许可证**: 如果 QtMqtt 不可用, 备选 Mosquitto 嵌入式客户端(EPL 许可)
5. **文件体积限制**: 严格遵守 .cpp ≤ 500行 / .h ≤ 200行, 复杂模块拆分为子模块

### 架构兼容性

所有新增连接类型(F12~F17, F20)统一实现 `IConnection` 接口:
- `type()` 返回新的 `ConnectionType` 枚举值
- `configure()` 接受 `QVariantMap` 参数, 每种连接类型自行解析所需字段
- `dataReceived` / `stateChanged` / `errorOccurred` 信号统一接入现有数据管道

所有新增协议类型(F18~F19)通过 `ProtocolEngine` (F2) 扩展:
- Modbus 作为内置协议模板
- Protobuf/FlatBuffers 作为自定义协议解码器

---

> **注意**: 本路线图是活文档, 随开发推进和用户反馈持续更新。
> 优先级可根据实际需求调整。每个特性在正式开发前需输出独立的 FEATURE_XXX.md 详细设计文档。
> 所有新增代码遵守 CLAUDE.md 中的铁律: 四层架构、IConnection 接口、文件体积限制、中文注释。
