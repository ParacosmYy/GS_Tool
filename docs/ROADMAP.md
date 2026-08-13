# 迭代路线与验收

## Milestone 0 — Python 工程与工作流

状态：已完成（代码、静态检查、启动和打包门通过；真实硬件和发行授权仍单独记录）。

- 建立 `domain/application/infrastructure/presentation` 包结构；
- 创建 PySide6 空工作区和组合根；
- 建立依赖、检查、打包和六角色 review workflow；
- 确认 Python 3.12、PySide6、pyserial、PyInstaller 的锁定策略；BLE 依赖作为可选 extra。

验收：不连接设备也能启动窗口、关闭干净、配置文件路径明确；UI 不直接依赖设备库。

## Milestone 1 — UART vertical slice

状态：代码切片完成，等待真实 USB-UART/回环硬件验收。

- COM 端口枚举、VID/PID/序列号/设备路径展示；
- 波特率、数据位、校验、停止位、流控和超时；
- 文本/Hex 终端、发送历史、快捷命令和暂停滚屏；
- 后台 worker、取消、热插拔身份诊断；不做未经确认的自动重连；
- 原始 JSONL 记录、发送历史、快捷命令和错误/丢弃计数；
- 读写超时保持有限值，记录队列同时受记录数和 16 MiB 字节上限约束，保证关闭路径可控。

验收：真实 USB-UART 或授权回环可以收发；拔插、乱码、关闭和高频日志可诊断；UI 线程不阻塞；PyInstaller EXE 能在干净 Windows 启动。当前未运行的项目是硬件收发、拔插和干净 Windows 验收。

## Milestone 2 — TCP/UDP

### M2a — TCP Client + UDP 单播（代码完成，loopback/打包门通过）

- 单活动 TCP Client，有限连接/读写超时、明确 EOF/断线状态、手动恢复；
- 单活动 UDP 单播，固定远端、显式本地绑定、datagram 边界和 peer 元数据；
- TCP/UDP 复用同一终端、原始记录、发送历史、快捷命令和暂停预览；
- 使用标准库 socket，不引入 TCP Server、多客户端、广播或协议级统计。

验收：TCP 半包/粘包和 EOF/timeout 可区分；UDP 多报文边界、非法 peer、超限包和
本地发送失败可观察；网络传输不修改终端组件。

### M2b — TCP Server 单客户端（代码完成，loopback/打包门通过）

- 默认只监听回环地址；LAN 监听需要显式确认和非空 allowlist；
- 单客户端准入、拒绝原因、监听/停止状态和手动关闭；
- 首个 client EOF 后回到监听态；第二个 client 不进入终端、发送队列或原始记录；
- 独立 `TcpServerSessionManager` 和 `TcpServerTransportPort`，不扩张 M2a client/UDP manager。

M2b 当前只支持 IPv4 literal/CIDR allowlist；hostname、IPv6、多客户端和广播留给后续。

### M2c — TCP Server 多客户端（代码完成，loopback/offscreen 门通过）

- 有界 client 数 1–16，默认 4；每次 accepted connection 生成独立 UUID `PeerId`；
- 每个 peer 有独立出站队列、transport in-flight partial write 和总字节预算；
- 明确发送目标，不隐式广播；拒绝、写失败和断开只影响对应 peer；
- 单 worker 独占 listener/client/wake socket；取消只设置停止标志并唤醒 worker，最终句柄由 worker 释放；
- accepted socket 保持 non-blocking，按读写就绪和 round-robin 轮询，完整写入后才产生 TX 记录；Windows 不使用 `SO_REUSEADDR`。

验收：两个 localhost client 可以独立收发；第三个 client 在容量满时得到 busy；旧 `PeerId` 在断开重连后不能命中新连接；Qt offscreen 多 peer 必须显式选择目标，目标断开不自动切换；worker 在关闭预算内退出。未运行真实 LAN、防火墙、多网卡和压力测试。

丢包/乱序统计延后到协议里程碑；没有应用层序列号时，只统计本地丢弃、非法来源、
超限和发送失败，不能把 UDP 的网络损失伪装成可观测事实。

## Milestone 3 — BLE GATT

状态：代码完成；BLE extra、静态检查、无硬件启动/关闭和 BLE-enabled onedir 打包门通过；真实 BLE 实机验收待硬件和授权环境。

- 手动扫描/过滤、服务发现、特征属性展示；
- read、write-with-response、write-without-response、notification/indication；
- 配对选项、缓存 service 选项、MTU 和 without-response 写长提示；
- Windows BLE dedicated asyncio worker 与 Qt 生命周期隔离；断线后错误可见，需手动重新扫描和连接，不自动重连。

验收：至少一个真实 BLE GATT 设备通过实机验证；需覆盖扫描、服务/特征发现、读、两种写模式、通知、断线和必要时的配对/重新授权。当前未运行这些实机项目；Bluetooth Classic SPP 不在这一里程碑内。

## Milestone 4 — 协议与可视化

### M4a — 有界 framing/checksum 与组件预览（代码完成，非硬件门通过）

状态：代码、静态检查、协议 worker 手工向量、offscreen 启动/关闭已通过；真实设备吞吐、长时间压力和发行验收待后续。

- UART/TCP Client 接收方向支持 Raw、Line（LF/CRLF）、Delimiter、Length prefix；
- 支持固定 preset 的 XOR-8、CRC16 Modbus、CRC32/IEEE；校验失败帧仍显示 Hex 和原因；
- parser 使用独立 bounded worker，按 `(session, peer/peer_id, channel, direction)` 隔离 partial state；原始记录和终端路径不受 parser 丢弃影响；
- 主窗口加入协议配置和有界组件帧预览；配置/重置只影响 parser，不重连、不改变发送、不改变 JSONL raw schema；
- UDP、TCP Server 多 peer、BLE notification/read 在 M4a 只走原始终端/记录，保留 source-aware pipeline 作为后续扩展边界。

- 未完成：profile schema/迁移、JSON/TLV/字段 codec、数据表、曲线、过滤器、回放和导出。

### M4b — profile/codec 与可视化（代码完成，非硬件门通过）

状态：严格 JSON profile、固定字段 codec、bounded component worker、表格/过滤/CSV 基础已通过代码、静态、手工、offscreen 和发行启动门；曲线、回放和迁移仍后置。

- profile schema v1：固定 offset/length、Hex/UTF-8/有符号/无符号/Float32、端序、scale、unit；
- profile 只允许声明式字段，不执行 Python、表达式、命令或 DLL；文件和字段数量均有上限；
- parser frame 经独立 component worker 生成字段结果，raw recorder 和协议帧不依赖字段 codec 成功；
- UI 提供 JSON profile 显式加载、bounded table、有效/错误过滤和 CSV 导出；
- 未完成：通用协议 codec、曲线、字段绑定 transform、profile 迁移、回放和大型数据集。

## Milestone 5 — 高级能力

### M5a — schema v2 JSON/TLV RX codec（代码完成，非硬件门通过）

状态：代码、静态检查、inline vectors、worker/offscreen 和默认 onedir/onefile 启动门通过；
真实设备吞吐、全传输覆盖和迁移仍后置。

- 旧 profile schema v1 保持原 JSON 形状和 `BinaryComponentCodec` 语义，不自动改写；
- schema v2 以独立 `codec` 块声明 JSON 或 TLV 版本，内置 codec 通过显式 router 选择；
- JSON 首版只支持严格 UTF-8、有限 JSON Pointer（无 wildcard、filter、表达式或脚本）、string/number/boolean/json 字段；
- TLV 首版只支持平面 `tag | length | value`，tag/length 宽度 1/2/4、显式端序、length 只表示 value，无嵌套和自动重同步；
- 缺失字段、类型不符、重复/未知 tag、截断和超限保持 row/field/codec error 可见，不关闭会话、不改变 framing、发送或 raw JSONL；
- 本轮只把 JSON/TLV codec 接入现有 UART/TCP Client RX component bridge；UDP、TCP Server、BLE、RTT 继续 raw-only；
- 使用 Python 标准库 `json`/`struct`，不新增运行时依赖，不执行用户代码。

### M5b — typed transform 与 bounded dataset（代码完成，非硬件门通过）

状态：代码、静态检查、inline vectors、worker 生命周期、Qt offscreen 启动/关闭已通过；
默认 onedir/onefile 需要在本轮最终变更后重建，真实设备吞吐和曲线仍后置。

- `ComponentFieldValue` 保留有限 typed `value`，显示字符串不作为后续计算输入；
- Dataset 使用独立 schema v1，series 选择组件字段并声明有限 `scale`、`offset`、`clamp`、`enum` 链；
- 每条 chain 最多 8 项，enum 只能是最后一项，未知枚举值和 transform 异常保持 warning/error 可见；
- Dataset worker 使用 128 项/256 KiB 有界队列和最大 1024 条内存窗口，重配置按 generation 丢弃旧任务；
- UI 提供 Dataset JSON 显式加载、sample 预览、统计和有界 CSV 导出；配置错误不影响 raw、framing、发送或会话；
- 不新增运行时依赖，不执行表达式、脚本、动态导入、TX 编码、数据库或图表运行时。

### M5c — 历史 RX 回放（代码完成，非硬件门通过）

状态：严格 JSONL 回放、协议/组件/Dataset 复用、暂停/恢复/停止、坏记录可见、静态检查、
手工向量、Qt offscreen 启动/关闭已通过；最终 onedir/onefile 发行门已在本轮变更后重建，
真实设备吞吐、长时间压力和曲线视图仍后置。

- 回放只读取当前 recorder JSONL 的 RX 原始记录，不连接设备、不发送 TX、不重新录制；
- `ReplayRecordCodec` 严格校验字段集合、重复 key、时间戳、UUID、base64、时区和 payload 上限；
- 回放 worker 使用有界行读取、文件/记录预算、历史 `DataOrigin` 和稳定 replay UUID，不能伪装成实时会话；
- 回放时间轴使用捕获时间差与 monotonic 调度，暂停冻结时间，协议队列背压会重试而不是静默丢弃；
- 历史 RX 通过既有 `ProtocolPipelinePort → ComponentPipelinePort → DatasetPipelinePort`，不复制传输逻辑；
- UI 暂提供速度、选择并回放、暂停、停止和状态/计数预览；曲线/时间轴组件订阅 DatasetBatchEvent 留到后续切片；
- 继续保持 J-Link RTT 最后接入，当前不因回放切片提前引入 J-Link 驱动或 SDK。

### M5d — 有界 Dataset 曲线监视（代码完成，非硬件门通过）

状态：presentation-only 自绘 Qt Widgets 曲线、series 选择、历史来源标记、错误/跳过计数、
100 ms latest-wins 刷新、手工投影向量和 Qt offscreen 渲染已通过；最终发行包需随本轮变更重建。

- 只显示用户选中的一个 Dataset series，避免不同单位共用误导性 Y 轴；后续多曲线另行定义；
- X 轴使用当前有界窗口内的相对 `occurred_at`，不显示绝对 monotonic/wall time；
- 只绘制有限 `int/float` typed value；bool、字符串/enum、缺失字段和 transform error 保留在 Dataset 文本预览但不绘为 0；
- 曲线点上限 512，且不超过 Dataset 当前 capacity；Widget 只接收 immutable `CurveSnapshot`，不访问 worker/port；
- 100 ms 合并绘制请求，Dataset/event pipeline 继续按 50 ms Qt poll 和有界窗口运行；
- 历史/实时 source 不混画；停止回放立即阻止迟到历史派生事件，但保留已绘制窗口；
- 不引入 QtCharts、pyqtgraph、数据库、字体资源、脚本、告警、游标、重采样或新运行时依赖；J-Link RTT 仍最后。

### M5e — 有界声明式批量命令（代码完成，非硬件门通过）

状态：固定步骤、显式执行/停止、历史复用、Qt 编辑器、静态检查、手工向量、offscreen 启动/关闭和默认双模式打包已通过；真实 UART/TCP/UDP/BLE 吞吐、错误链路和干净 Windows 仍待授权环境。J-Link RTT 批量发送不在本轮，RTT 继续最后。

- 进程内最多 16 个宏；每个宏 1–32 步；单步最终 payload（含 CRLF）最多 512 B，总 payload 最多 16 KiB；
- 步骤间固定延时 0–2,000 ms，总延时最多 180 s；顺序执行，不支持脚本、循环、条件、变量、嵌套、ACK 等待或自动重试；
- UART/TCP Client 使用 stream，TCP Server 必须绑定显式 `PeerId`，UDP 保留 datagram 边界，BLE 必须绑定 characteristic 和 write mode；不广播、不自动切换目标、不自动分片；
- 运行前先构造全部 typed write；后台 worker 逐条调用现有 `SessionPort.send()`，延时可取消，状态语义为“已提交到本地发送队列”，不冒充线缆完成；
- 每个已接受步骤沿用单条发送历史和 raw recorder；回放期间禁用批量命令；宏仅在当前进程内存保存；
- 关闭先停止批量 worker，再关闭 session；不新增运行时依赖，不引入脚本引擎或 J-Link 驱动/SDK/DLL。

### M5g — NMEA 0183 RX line checksum profile（代码完成，非硬件门通过）

状态：公开来源适用性记录、独立 NMEA line checksum、坏格式/坏校验可见、静态检查、手工向量、
Qt offscreen 和默认双模式打包已通过；真实 GNSS/UART、NMEA 标准版本覆盖和字段 codec 仍后置。

- 复用既有 Line (LF/CRLF) framing；NMEA checksum 模式只允许 Line，不增加 transport 分支；
- 要求 `$` 开始、唯一 `*`、非空 ASCII sentence body、尾部两个 ASCII hex 字符；checksum 是 `$` 与 `*` 之间字节的 8-bit XOR；
- 格式错误使用 `INVALID_FORMAT`，数值不匹配使用 `INVALID_CHECKSUM`；坏帧仍进入协议/component 可见路径，原始 terminal/recorder 不变；
- valid component payload 去除 `*HH` 后缀，便于后续文本字段 profile；当前不解析 RMC/GGA/AIS 字段、不做 TX checksum、ACK 或标准版本全覆盖；
- NMEA 官方标准正文受版权保护；当前实现依据公开设备规格记录的线格式作为工程参考，真实设备仍须按设备/授权标准手册确认；
- 不新增运行时依赖，不引入 NMEA 第三方库、动态脚本或 J-Link 驱动/SDK/DLL。

### M5h — Modbus RTU 已分帧 ADU RX validator/profile（代码完成，非硬件门通过）

状态：公开 Modbus 来源适用性、独立 `domain.modbus`、schema v2 component codec、坏协议字段阻断
Dataset、静态检查、inline vectors、Qt offscreen、双模式打包和启动/退出已通过；真实 UART 仍后置，
t1.5/t3.5 timing framer 和完整主从事务后置。

- RTU ADU 按 `address | function | data | CRC low | CRC high` 校验，最大 256 B，data 最大 252 B；
  地址 0–247 的规则、保留地址、function 基本格式和 CRC mismatch 都以可见 row/status/error 发布；
- schema v2 `modbus_rtu` codec 默认展示 address、function、exception、data length/data Hex、
  exception code 和 received/calculated CRC；字段声明是有限 `name/source`，不执行用户代码；
- codec 只消费已经分帧的完整 `DecodedFrame.payload`，不修改 transport、raw recorder、terminal 或
  通用 framing；由于通用 checksum 会剥离尾部 CRC，Modbus CRC 的归属必须保持单一；
- 坏 checksum/格式/截断/超限字段保留 raw 并阻止 Dataset 计算；不做 t1.5/t3.5、自动重同步、
  request/response 关联、主从事务、ASCII、完整 function-code/PDU 字段语义、寄存器映射或 TX；
- 官方参考：[Modbus Serial Line V1.02](https://www.modbus.org/file/secure/modbusoverserial.pdf)、
  [Modbus Application Protocol V1.1b3](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)。

### M5i — MAVLink v1/v2 已分帧 RX validator/profile（代码完成，非硬件门通过）

状态：公开 MAVLink 来源适用性、独立 `domain.mavlink`、schema v2 component codec、CRC_EXTRA
缺失/CRC 错误/签名未认证状态、静态检查、inline vectors、Qt offscreen 和双模式打包已通过；
真实 UART/飞控、方言完整映射和签名密钥验收仍后置。

- v1/v2 packet 按 magic、header、payload length、sysid/compid、v2 incompat flags 和有界 packet
  长度校验；v2 signed packet 保留 13 B signature，但不进行认证；
- CRC-16/MCRF4XX 使用 profile 的显式 `message_id -> CRC_EXTRA` 映射；映射缺失或 signature
  未认证都发布 `UNVERIFIED`，坏 wire CRC 发布 `INVALID_CHECKSUM`；
- schema v2 `mavlink` codec 只提供有限 packet metadata/payload/signature Hex 字段，字段错误
  阻止 unverified/bad packet 进入 Dataset；映射来源和 revision 必须在 profile 中记录；
- codec 只消费一个已经分帧的完整 `DecodedFrame.payload`，不把一次 UART/TCP read 当作 MAVLink
  packet，不实现 stream resync、完整 dialect/message payload 字段、TX 编码或签名密钥管理；
- 示例 `profiles/mavlink-common-heartbeat.json` 使用官方 common message id 0/CRC_EXTRA 50，
  但 `master` 只是开发示例，生产前必须固定官方 message-definition revision；
- 官方参考：[MAVLink overview](https://mavlink.io/en/about/overview.html)、
  [packet serialization](https://mavlink.io/en/guide/serialization.html)、
  [message signing](https://mavlink.io/en/guide/message_signing.html)、
  [common.xml](https://raw.githubusercontent.com/mavlink/mavlink/master/message_definitions/v1.0/common.xml)。

### M5j — pipeline generation barrier（代码完成，非硬件门通过）

状态：六角色独立审查、跨阶段 generation 传播、下游 stale-event fence、ViewModel 链式过滤、
线程级 inline vector、静态检查和 Qt API 兼容性检查已通过；真实 UART/BLE/RTT、Replay session
segment 和正式发行门仍后置。

- `ProtocolFramesDecodedEvent` 携带 protocol generation；component/Dataset event 同时携带自身
  generation 与上游 generation；backpressure 事件也绑定所属 generation；
- protocol worker 在重配置/reset 时丢弃 pending ingress；component/Dataset worker 在入队前按
  upstream fence 拒绝旧事件，在 decode/commit/publish 前继续检查本阶段 generation；
- ViewModel 在 protocol、component、Dataset 变更后同步 generation，并要求完整 generation chain
  匹配后才更新 rows、stats、samples 和曲线输入；这不是只隐藏 UI，而是阻断后端派生状态污染；
- 不修改 transport、raw recorder、队列容量、协议 codec 或 RTT；不引入锁外全局状态、动态插件或
  新运行时依赖；
- 未在本轮实现 Replay 原始 session segment reset、已分帧 codec 的实时 stream boundary、统一
  local error 状态和发行许可证清单，它们保留独立边界。

### M5k — replay session segment reset（代码完成，非硬件门通过）

M5k 处理同一 JSONL 回放文件中相邻原始连接 session 的 parser partial buffer 隔离。`RawRecord`
已有原始 `session_id`，本轮把它作为 `ProtocolIngressUnit.segment_id` 的 parser-only metadata；
replay 仍使用独立 replay UUID 作为下游 source identity。

- replay worker 只给历史 stream ingress 标注原始 session，不修改 recorder、`ReplayDataEvent` 或
  component/Dataset event；
- protocol worker 在 FIFO 出队处发现 segment 变化时，先对旧 source `finish()`，发布可见的
  `INCOMPLETE` tail，再 reset 旧 decoder，最后 feed 新 segment；这样不在 replay 线程直接 reset，
  不丢已接受队列，也不引入跨层 session 分支；
- segment map 与 parser decoder 一起受 configure/reset 清理，连续重复出现同一 UUID 也按连续段
  变化处理；
- 统计、组件和 Dataset 仍是同一 replay run 的 bounded preview，segment selection/UI 展示另列；
- 未在本轮实现实时 Modbus/MAVLink timing/resync、local error、真实 UART/BLE/RTT 或正式发行门。

### M5l — presentation-owned unified error（代码完成，非硬件门通过）

M5l 消除 MainWindow 本地校验直接写 QLabel、ViewModel 结构化错误走另一条路径造成的双写状态。

- `SessionViewModel` 唯一持有 `ErrorInfo | None`，通过 `error_info`/`error_message` 暴露只读摘要；
- `show_local_error()` 使用现有 `ErrorCode.CONFIGURATION` 包装本地校验，不新增 domain 错误码或
  transport 分支；
- `error_changed` 统一发送结构化错误或 `None`，MainWindow 只渲染消息、detail tooltip 和可见性；
- 成功操作、显式清除和会话/回放生命周期沿用 `_clear_error()`，不会留下第二套 local QLabel 状态；
- 未在本轮实现错误队列、按 session 的多槽聚合、实时 Modbus/MAVLink timing/resync、真实 UART/BLE/RTT
  或正式发行许可证门。

### M5m — realtime stream boundary（代码完成，GUI/硬件门后置）

M5m 把已分帧的 Modbus/MAVLink validator 前移到 source-aware stream boundary。后端 DTO、
边界 decoder、queue continuity barrier、MainWindow 配置接入、静态检查和进程内 bytes vectors
已完成；GUI offscreen、真实时序/硬件和正式发行门仍后置，不能把代码完成宣称为完整产品验收。

- `ProtocolIngressUnit` 携带带质量的 `GapObservation`；当前 session 只记录 host read gap，
  明确不等价于线缆上的每字节 timestamp；历史 replay 没有该时序事实，不会伪造；
- MAVLink stream 按 FE/FD、长度、signed signature 长度和有界 packet size 提取，噪声、坏长度、
  partial 和 resync/drop 统计可见；没有 CRC_EXTRA/profile 时是 `UNVERIFIED`，显式 profile
  仍可在 component 阶段升级/拒绝；
- Modbus RTU timed stream 只接受带 timing quality 的 gap；t3.5 结束当前 ADU，t1.5～t3.5
  发布 `INCOMPLETE` 并清理，普通 read gap 不会触发 wire boundary；完整 ADU 继续由独立
  `domain.modbus` codec 校验；
- protocol ingress queue drop 后按 source 建立 continuity epoch，在 worker FIFO 处先结束旧
  partial 再处理新 epoch，避免丢失字节跨段拼帧；raw terminal、recorder、transport 和发送
  路径不变；
- `ProtocolStats` 增加 incomplete、gap boundary、resync；MainWindow 已与 valid/invalid、buffer、
  parser drop 分层展示，并在 Modbus framing 下显示 host-gap 诊断限定；动态二次元背景属于
  presentation-only UI-1，必须可暂停、低动效和资源失败回退。

UI-1 当前已先完成方向无关的 presentation 基础层：集中式主题 token、连接状态区、连接/协议/历史
分页、终端主路径、曲线 token、分区标题/空态/只读结果表/键盘路径，以及共享动效控制器驱动、独立的
“暂停动效/低动效”状态脉冲和顶部信号场。二次元视觉候选、背景资源、GUI/offscreen 和重新打包仍是
后续门；在用户选择视觉方向前不固化具体美术资产。

M5m 后置：断开时 partial 的最终产品呈现；真实 UART per-byte/hardware timing、Modbus 主从事务、
MAVLink dialect/signature 认证、GUI offscreen/干净 Windows 和正式发行门。J-Link RTT 仍最后。

### G0 — Windows packaging provenance and variant isolation（已完成，非正式发行门）

G0 将打包收敛为唯一的 `scripts/package.ps1` 入口，并把版本、能力变体和模式纳入输出边界。

- core/onedir、core/onefile、ble/onedir、ble/onefile 分别写入
  `dist/release/<version>/<variant>/<mode>`，不共享 staging 或覆盖彼此产物；
- 从唯一版本源生成 PE version resource，记录 source revision、`uv.lock` hash、toolchain、
  artifact SHA256、PE 字段、签名状态、硬件验收状态和许可证清单状态；
- 每个模式保存 PyInstaller archive listing、locked dependency tree、NOTICE、第三方清单和
  可重复的 manifest verification；core 内容门拒绝 Bleak/WinRT，所有变体拒绝 J-Link/SEGGER/probe-rs
  vendor binary；
- 当前包仍是 unsigned engineering build，许可证文件只是 inventory，`release_eligible=false`；
  RTT 仍是最后阶段，不因打包切片引入驱动、SDK、DLL 或厂商工具。

G0 验收：四个矩阵包、manifest/hash/archive gate、PE metadata、NotSigned 状态和 GUI 启动/关闭；
没有 COM/BLE 实机或完整法律许可证 bundle 时，不宣称硬件或正式发行通过。


- Modbus RTU timing/transaction、MAVLink 完整 dialect/profile 等更深协议能力；
- 独立进程或 WASM 插件隔离；
- 固件烧录、完整调试器和签名发布。

这些能力不得提前阻塞前五个里程碑。

## Milestone 6 — J-Link RTT（最后，代码完成/非硬件门通过）

J-Link RTT 明确放在所有基础传输和协议能力之后。本切片采用 attach-only：SerialForge
只连接用户已经启动的 RTT Telnet 服务，不安装、启动或托管 SEGGER 工具，不把驱动、SDK
或 DLL 作为主线依赖。

- 默认连接 `127.0.0.1:19021`，主机和端口可配置；
- 通过官方 RTT Telnet Config String 选择 channel 0/1，并复用有界 stream/session、终端、发送和原始记录；
- 连接失败、EOF、超时和关闭沿用结构化 transport/session 错误；不把普通 TCP 连接错误武断解释成探针或目标诊断；
- 不提供自动重连、探针/目标发现、RTT Viewer/Logger、memory/halt/run/reset/flash 或任意 J-Link 命令。

非硬件验收：标准库 TCP 回环验证 Config String、Up bytes、Down bytes、channel 1 和关闭边界；
静态检查、offscreen 启动和 PyInstaller 包门沿用最终轮验证。真实验收仍需使用已授权
J-Link/目标板，覆盖活动 debug session、目标 RTT 初始化、channel、日志吞吐、Down buffer、
断开和工具/驱动版本；不宣称完整源代码调试、通用内存操作或烧录。

## 每轮交付证据

记录检查命令、启动结果、EXE 路径、设备型号/驱动/参数、手工结果和未运行验证。没有真实硬件时，只报告静态、启动和打包证据。
