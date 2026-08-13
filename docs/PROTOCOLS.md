# 传输与协议设计

SerialForge 的“调试”不应只等于把所有输入当作文本。设备数据需要先保存原始字节，再按用户选择的解析链生成可视化数据。

## 传输类型

| 类型 | MVP 行为 | 关键差异 |
|---|---|---|
| UART | COM 枚举、参数配置、收发 | 连续字节流；可能有丢字节、乱码和热插拔 |
| BLE GATT | 手动扫描/过滤、服务发现、读写、通知 | Windows Central；MTU、特征属性、写模式和通知状态可见 |
| TCP | Client/Server | 连续字节流；必须处理半包、粘包和断线；Server M2c 支持有界多 client 与显式 `PeerId` 目标 |
| UDP | M2a 单播固定远端，后续广播 | datagram 有边界；先记录 peer、本地丢弃和发送失败；应用层序列号到位后再统计网络丢包/乱序 |
| J-Link RTT | 通道读取、发送、保存 | 依赖目标 RAM 中的 RTT 控制块和探针访问 |
| Bluetooth Classic SPP | 后续适配 | 与 BLE GATT 不是一套 API，Windows 专用支持另行评估 |

“Wi‑Fi 调试”在 PC 侧通常落到 TCP/UDP/WebSocket/MQTT 等网络协议。第一版选择 TCP/UDP，避免在还没有稳定会话模型时同时引入消息代理、证书和云端配置。

TCP Server M2c 的监听器使用 IPv4 literal bind，最多 16 个 client，默认 4 个。默认
`127.0.0.1` 不要求 allowlist；非回环监听必须显式确认 LAN 且配置非空 IPv4/CIDR
allowlist。每次 accept 生成新的 UUID `PeerId`；`PeerAddress` 只用于显示、allowlist 和
审计，不作为发送队列或重连身份。listener 和 accepted client 的生命周期分开表达：
client EOF、写失败、非法来源和 busy 不结束 listener；多 client 的每次发送必须携带明确
`PeerId`，没有广播、自动转投、自动重连、TLS 或 IPv6。accepted socket 由 server worker
独占，使用 non-blocking partial write；只有完整写入后才记录 TX 事件。

BLE GATT M3 只实现单设备 Windows Central：扫描结果是不可变设备快照，用户显式选择后再
连接；不会自动重连或把旧设备选择静默替换成新设备。服务和特征以不可变快照进入 UI，
特征属性至少区分 `read`、`write`、`write-without-response`、`notify` 和 `indicate`。
写命令必须携带 service UUID、characteristic UUID、handle、payload 和显式写模式；
without-response 受 backend 报告的最大写长约束，with-response 也有应用上限，M3 不做自动
分片。read 返回的 payload、notification/indication payload 和成功写入均带特征 channel
上下文；通知回调不得直接触碰 Qt。

配对、授权、设备断线、MTU/写长超限和不支持的特征通过结构化错误回到 UI。Bluetooth
Classic SPP 不复用 GATT 接口，J-Link RTT 在基础传输和协议切片完成后以最后能力接入。

## 帧解析链

建议把解析器组合成可观察的 pipeline：

```text
raw bytes
  -> capture/timestamp
  -> framing (raw | line | delimiter | length)
  -> integrity (none | XOR | CRC16 | CRC32)
  -> codec (UTF-8 | Hex | JSON | CBOR/TLV | binary schema)
  -> transform (scale | offset | EMA | unit)
  -> dataset/widget
```

首批内置模式：

- Raw/Hex：永远可用，且保留不可解码字节；
- Line：支持 `\\n`、`\\r\\n` 和自定义终止符；
- Length-prefixed：长度字段位置、端序和基准可配置；
- Delimiter：适合 COBS、SLIP 前后的包边界；
- JSON Lines/CSV：适合快速接入传感器和日志；
- TLV/二进制字段：支持无符号/有符号整数、浮点、端序、缩放和单位；
- CRC/XOR：校验失败的帧仍可显示为错误帧，不应悄悄丢弃。

Modbus、MAVLink、CBOR、MessagePack 等完整协议可作为后续 profile/codec；NMEA 当前先提供 line
checksum profile，字段语义仍后置。不要为每种协议在 UI 中复制一套终端逻辑。

### M5f 通用协议预设边界

M5f 提供 domain 层不可变 `ProtocolPreset` 目录，把常用的通用 `ProtocolConfig` 填入现有
协议编辑器。当前预设包括 Raw、LF/CRLF Line、AA 55 Delimiter、U8 little-endian length
prefix、U16 little-endian length prefix + CRC16/Modbus，以及 NMEA 0183 Line + XOR。预设选择只更新编辑区，必须由
用户显式点击“应用”才会调用现有 `configure_protocol()` 并清空 parser/component/dataset 的派生状态。

预设目录不依赖 Qt、transport、设备库或动态导入，也不等于实现了完整标准协议。Modbus RTU 的
地址/功能码/RTU 间隔，MAVLink 的版本/签名/CRC-extra，NMEA 的 RMC/GGA/AIS 字段语义，仍应在后续
独立 codec/profile 中实现并单独验证；不能用当前 preset 名称掩盖这些语义。

### M5g NMEA 0183 RX checksum 当前实现边界

M5g 只把 NMEA 0183 的一行作为已分帧 payload 处理。`NMEA0183` checksum 模式要求 Line
framing，并要求 sentence 以 `$` 开始、包含唯一 `*`、`$` 与 `*` 之间有非空 ASCII 内容，尾部
正好两个 ASCII hex 字符。校验值是 `$` 与 `*` 之间字节的 8-bit XOR；valid 派生 payload 去除
`*HH`，错误帧仍以 `INVALID_FORMAT` 或 `INVALID_CHECKSUM` 发布。原始 `StreamDataReceivedEvent`
和 JSONL recorder 不做修改，因此坏 checksum 的完整 wire bytes 仍可审计。

这一切片依据 [NMEA 官方页面](https://www.nmea.org/nmea-0183.html) 的标准归属/版本边界，以及
[Quectel LC29H(BS) GNSS Protocol Specification V1.0](https://www.quectel.com/content/uploads/2024/02/Quectel_LC29HBS_GNSS_Protocol_Specification_V1.0.pdf)
的设备实现参考；后者是适用于该设备的工程参考，不是所有 NMEA 设备的制造商要求。NMEA 标准
正文受版权保护，仓库不复制句型表或完整标准文本。当前不做 RMC/GGA/AIS 字段解析、TX 编码、
自动校验补全、NMEA 版本/方言全集、RTCM 混流或认证声明。

### M4a 当前实现边界

M4a 只把 UART/TCP Client 的 RX 接入组件解析入口；UDP datagram、TCP Server peer 和 BLE
GATT notification/read 仍保持原始终端/记录路径。parser worker 按
`(session_id, transport, peer/peer_id, channel, direction)` 隔离状态，不会把不同 peer 或 characteristic
的残帧拼接在一起。

- Raw：每个原始输入片段显示为一项，不启用 checksum；
- Line：以 LF 为终止符，CRLF 的 CR 不进入 payload，跨读取边界持续拼帧；
- Delimiter：用户输入非空 Hex 分隔符，分隔符不进入 payload，相邻分隔符可产生空帧；
- Length prefix：长度字段为 1/2/4 字节、显式大小端，长度表示 payload，不允许超过 65536 B；
- Integrity：XOR-8、CRC16/Modbus、CRC32/IEEE，校验字段固定在帧尾，字节序显式使用
  little/big；失败帧保留 payload Hex 和 expected/actual 原因；
- parser 输入队列、partial buffer、frame preview 均有上限。队列丢弃只影响组件预览，原始
  JSONL 记录和终端 raw bytes 不受影响；配置/重置不会改变连接或发送。

当前不做：任意 CRC 多项式、长度字段偏移/整帧长度基准、profile 保存迁移、数据表/曲线、
回放导出、TX 自动编码、自动重同步以及 Modbus/MAVLink 完整 codec、NMEA 字段 codec。schema v2 JSON/TLV codec
在 M5a 单独接入，不能改变 M4a framing/checksum 的职责。

### M5h Modbus RTU 已分帧 ADU RX codec 当前实现边界

M5h 新增独立 `domain.modbus` validator 和 schema v2 `modbus_rtu` component codec。输入契约是一个
已经由上游明确分帧的完整 `DecodedFrame.payload`；codec 不读取串口时钟、不操作 transport，也不从
普通 UART/TCP read chunk 推断 RTU 边界。默认字段是 unit address、raw/base function、exception 标记、PDU
数据长度、数据 Hex、exception code、received CRC 和 calculated CRC；字段可通过有限 `name/source`
声明缩减或重命名。

- ADU 结构按 `address | function | data | CRC low | CRC high` 校验，最大 256 B，数据最多 252 B；
  地址 0 保留为 broadcast，1–247 是设备地址，248–255 判为保留；function `0x00`/`0x80` 判为格式错误，
  其他 function byte（包括未知或异常响应）只展示，不假装拥有完整应用语义；
- CRC16 使用 Modbus 初始化和多项式，wire 低字节在前；格式错误、截断、超限和 checksum 错误保留 row，
  `FrameStatus` 与字段 error 可见；坏协议状态的字段不会被 Dataset 当作可计算输入；
- 已分帧前提以 [Modbus Serial Line V1.02](https://www.modbus.org/file/secure/modbusoverserial.pdf)
  为依据：RTU 的 t1.5/t3.5 静默间隔仍属于未来 timing-aware framer；当前不做自动重同步、request/response
  关联、主从事务、完整 function-code/PDU 字段语义、ASCII 模式、TX 或寄存器映射；
- 通用 framing 的 CRC 会剥离派生 payload 的尾部 checksum，因此使用本 codec 时不能再让上游通用
  checksum 消费同一段 Modbus CRC；raw terminal/recorder 仍保存事实 wire bytes。

应用协议的 function-code 与 exception 定义参考 [Modbus Application Protocol V1.1b3](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)，
但本切片只显示 function/exception code，不把未知设备的寄存器地址或 PDU 长度静默判断为合法业务。

### M5a schema v2 component codec 当前实现边界

M5a 只处理现有 UART/TCP Client RX 产生的 `DecodedFrame.payload`，是 component table 的派生
视图，不是原始线缆帧；原始字节仍由 terminal 和 JSONL recorder 保存。旧 profile schema v1
保持固定 offset/length 字段语义，新配置使用如下严格 v2 形状：

```json
{
  "name": "sensor-json",
  "schema_version": 2,
  "codec": {
    "kind": "json",
    "version": 1,
    "encoding": "utf-8",
    "fields": [
      {"name": "temperature", "path": "/sensor/temperature", "value_kind": "number", "unit": "C"}
    ]
  }
}
```

JSON path 是有限 JSON Pointer；只允许 literal object key 和数组下标，不支持 wildcard、filter、
slice、递归、表达式或脚本。payload 必须是严格 UTF-8 JSON，重复 key、NaN/Infinity、超深/超大
节点树和超长字符串会成为可见 codec error。字段缺失、null/类型不符不抛出 worker，保留 row
和 raw Hex。

TLV v1 的 wire layout 是 `tag | length | value`。`type_bytes`、`length_bytes` 只能为 1/2/4，
共用显式 `byteorder`，length 只表示 value 字节数；不支持嵌套、填充或自动重同步。字段可
声明 `tag`、`occurrence`、固定 `length`、Hex/UTF-8/uint/int/Float32、端序、scale 和 unit。
截断 header/value、未知 tag（按配置 ignore/error）、重复 tag 未完整声明 occurrence 和超限
数据均保留 raw 并显示 codec/field error。M5a 只做 RX decode，不自动生成 TX。

UDP datagram、TCP Server peer、BLE read/notification 和 RTT channel 暂不进入该 component
worker；它们保留各自的 raw boundary/context，后续扩展必须分别定义 source/channel 语义。

### M5i MAVLink v1/v2 已分帧 RX validator/profile 当前实现边界

M5i 新增独立 `domain.mavlink` validator 和 schema v2 `mavlink` component codec。输入契约是一个
已经由上游明确分帧的完整 `DecodedFrame.payload`；codec 不读取串口时钟、不操作 transport，也不从
普通 UART/TCP read chunk 推断 MAVLink packet 边界。字段默认展示 version、payload length、v2 flags、
sequence、sysid、compid、message id、signed、CRC_EXTRA、received/calculated CRC、payload Hex 和
signature Hex。

- v1 头部为 `FE`，v2 头部为 `FD`；v2 signed incompat flag 会要求并保留 13 B signature。未知
  incompat flag、header/payload/CRC 截断、多余字节、sysid/compid 0 和超限 packet 都发布可见状态；
- CRC 使用 MAVLink 的 CRC-16/MCRF4XX 算法，计算范围为 packet 的 length/header remainder + payload +
  profile 提供的 CRC_EXTRA，wire CRC 按小端读取；没有 message-id 映射时不会猜测 CRC_EXTRA，状态为
  `UNVERIFIED`；
- wire CRC 正确的 signed v2 仍是 `UNVERIFIED`，因为本切片不做 signature authentication；坏 wire CRC
  是 `INVALID_CHECKSUM`。所有非 `VALID` 状态的派生字段带 error，因此 Dataset 不会把它们当作计算输入；
- profile 记录 `dialect`、`mapping_source` 和 `mapping_revision`。示例使用 common message id 0/
  CRC_EXTRA 50，但 `master` 仅为开发示例，生产前必须固定官方 message-definition revision；
- 依据 [MAVLink overview](https://mavlink.io/en/about/overview.html)、
  [packet serialization](https://mavlink.io/en/guide/serialization.html)、
  [message signing](https://mavlink.io/en/guide/message_signing.html) 和官方
  [common.xml](https://raw.githubusercontent.com/mavlink/mavlink/master/message_definitions/v1.0/common.xml)。
  这些是公开协议/实现参考，不是特定飞控厂商的制造商要求；不复制方言全集；
- 当前不做 stream resynchronisation、完整 dialect/message payload 字段生成、signature key 管理、
  TX 编码、request/response 或飞控命令语义；TCP/UDP/BLE/RTT 的 component scope 不因该 codec 扩张。

### M5m 实时 stream boundary 当前实现边界

M5m 在 M5h/M5i 的已分帧 codec 之前增加独立、按 `ProtocolSource` 隔离的 stream boundary。它只
消费 application 提供的 `ProtocolIngressUnit`，不读取 socket、Qt 或 transport，并保持 raw
terminal/recorder 为事实来源。

- MAVLink `MAVLINK_STREAM` 使用 `FE`/`FD` magic、header length、v2 signed flag、13 B signature
  和 280 B packet 上限做结构提取；前导噪声、坏长度和有界 buffer overflow 会形成可见
  dropped/resync 统计。结构完整的 packet 先发布 `UNVERIFIED`，因为 boundary 没有 CRC_EXTRA
  方言映射或 signing key；既有 `mavlink` component profile 可在第二阶段升级为 `VALID` 或
  发布 `INVALID_CHECKSUM`；
- MAVLink 的 re-anchor 只在后续 magic candidate 已完整且结构可接受时触发，是保守的有界启发式，
  不等于 dialect 解析、CRC_EXTRA 验证或 signature authentication；
- Modbus `MODBUS_RTU_TIMED` 需要 `ModbusRtuTiming`，默认根据 UART bits/baud 计算 t1.5/t3.5，
  19200 baud 以上使用 750 µs/1750 µs 固定值。只有 `GapObservation.quality` 为
  `HOST_READ_GAP` 或 `DEVICE_TIMESTAMP` 时才使用 gap：大于等于 t3.5 结束候选 ADU，介于
  t1.5 与 t3.5 之间发布 `INCOMPLETE` 并清理，小于等于 t1.5 合并；没有 timing quality 的
  普通 `feed(bytes)` 只追加，不凭 read chunk 分帧；
- 当前 Windows session 只能生成 `HOST_READ_GAP`，所以该结果是诊断性 host scheduling 证据，
  不能证明在线缆上的每字节静默间隔。replay ingress 没有时序字段，不会伪造实时 timing；
- parser queue 丢弃会为 source 增加 continuity epoch，worker 按 FIFO 先结束旧 partial，再处理
  新 epoch，避免被丢弃的字节跨 gap 拼帧。`ProtocolStats` 的 incomplete、gap boundary、
  resync 和 dropped bytes 必须与业务 valid/invalid 分开显示；
- MainWindow 已接入两个新 framing、显式 preset application、当前 UART 派生的 Modbus timing
  提示，以及 incomplete/gap/resync/parser drop 统计；Modbus framing 只允许 UART，host read gap
  仍明确标记为诊断证据。Modbus 主从事务、request/response、寄存器映射、MAVLink 完整
  dialect/message field decode、signing verifier、TX 编码和真实 per-byte timestamp 均未实现。

实现依据与范围记录在 [Modbus Serial Line V1.02](https://www.modbus.org/file/secure/modbusoverserial.pdf)
及项目已有的官方 MAVLink 资料中；公开协议资料不是某一具体设备的制造商要求，真实 UART 仍须
在授权环境按设备手册验收。

### M5b typed transform 与 dataset 当前实现边界

组件字段结果同时保留 `raw`、有界 `display` 和有限 scalar `value`。Dataset 不从 display
字符串反向解析，而是选择 typed value 后执行声明式链：

```text
component field.value -> scale -> offset -> clamp -> enum -> DatasetValue
```

Dataset schema v1 示例：

```json
{
  "name": "sensor-window",
  "schema_version": 1,
  "capacity": 256,
  "series": [
    {
      "field": "temperature",
      "unit": "C",
      "transforms": [
        {"kind": "scale", "value": 0.1},
        {"kind": "offset", "value": -10},
        {"kind": "clamp", "minimum": -40, "maximum": 125}
      ]
    },
    {
      "field": "state",
      "unit": "",
      "transforms": [
        {"kind": "enum", "mapping": {"0": "Idle", "1": "Run"}}
      ]
    }
  ]
}
```

每条链最多 8 项，enum 只能是最后一项；scale/offset/clamp 的配置和结果必须是有限数字。
未知枚举值生成 warning，不会静默变成已知状态；字段缺失、上游错误和 transform 异常均保留
为 DatasetValue error。worker 使用 128 项/256 KiB 有界输入队列和最大 1024 条内存窗口，
重配置通过 generation 丢弃旧派生任务。Dataset 仅接收 UART/TCP Client RX component event，
不改变 framing、raw JSONL、终端、TX、UDP/TCP Server/BLE/RTT 边界；CSV 只是当前窗口的导出。

Replay parser 的 session boundary 独立于上述 source identity：历史 ingress 的
`ProtocolIngressUnit.segment_id` 使用原始 `RawRecord.session_id`，protocol worker 在 FIFO 顺序
中 finish/reset 旧 partial decoder 后再接收新 segment。Replay UUID、`DataOrigin.HISTORICAL`、
raw recorder 和下游 bounded preview 不因此增加新的 transport 或 codec 分支。

### M6 RTT Telnet 当前实现边界

RTT 采用 attach-only 外部桥接：用户先启动提供 RTT Telnet 服务的 J-Link Commander、GDB
Server 或 IDE，SerialForge 通过标准库 TCP 连接默认 `127.0.0.1:19021`。连接建立后在短
配置窗口发送 `$$SEGGER_TELNET_ConfigStr=RTTCh;<channel>$$`，当前受限为 channel 0/1；随后
Up bytes 和显式 Down bytes 复用已有 stream/session、终端和 raw JSONL recorder。

- 不启动 `JLink.exe`/`JLinkRTTClient.exe`，不加载 SEGGER SDK/DLL，不分发驱动或 vendor binary；
- 不提供 probe/target discovery、自动重连、RTT Viewer/Logger、memory/halt/run/reset/flash；
- RTT 首版只走原始终端和记录，不进入 M4a/M4b parser/component worker；
- socket/open/timeout/EOF/close 错误如实显示，不把它们推断为具体探针占用或目标未初始化；
- 多 Telnet 客户端、目标 Down buffer 消费/丢失、吞吐和断线必须在授权硬件上单独验收。

## 设备配置文件

配置文件只描述可复现的连接和解析，不存储秘密。概念示例：

```json
{
  "name": "board-uart-115200",
  "transport": {
    "kind": "uart",
    "match": { "vid": "0x0483", "pid": "0x374B" },
    "baud_rate": 115200,
    "data_bits": 8,
    "parity": "none",
    "stop_bits": 1,
    "flow_control": "none"
  },
  "decoder": {
    "framing": "line",
    "encoding": "utf-8",
    "terminator": "\\n"
  },
  "commands": [
    { "name": "version", "payload": "version\\n" }
  ]
}
```

实际实现时需要给 schema 加版本号、未知字段策略和迁移策略。BLE 配置应引用 service/characteristic UUID；当前 RTT 配置只引用 Telnet host/port、channel 和 timeout，不接受 probe selector、芯片/内存范围、command file 或 DLL 路径。

## 记录与回放

记录项至少包含：单调时间、墙上时间、会话 ID、方向、传输通道、原始 payload、解析状态和错误码。建议：

TCP 记录的是连续字节流片段，不能把一次 socket read 当作协议帧；UDP 记录必须保留
一次 datagram 的完整 payload 和来源/目标 peer。发送记录表示本地提交成功，不表示
远端应用已经处理；没有应用层序列号时不推断 UDP 网络丢包或乱序。

- 原始记录优先用 JSONL 或带二进制 payload 的自定义 append-only 格式；
- CSV 只作为扁平化导出，不作为唯一真相；
- 回放必须标注“历史数据”，不能伪装成实时设备；
- 记录器有最大文件大小、滚动策略和丢弃计数；
- UI 应能查看解析失败的原始 Hex。
