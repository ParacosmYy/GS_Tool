# PRD_092 - Serial Station Log Service

## 1. 背景

Serial Station 已经具备 UART 配置、发送闭环、接收闭环、Dispatcher、ASCII/Custom MD/Modbus RTU 协议能力，但日志仍主要停留在 `SerialLogPanel` 的 UI 内存字符串中。

这会带来三个问题：

- UI 面板承担日志存储、过滤和展示状态，后续导出、回放、审计无法复用。
- Controller 只能向 UI 发散字符串信号，缺少统一的结构化收发记录。
- 日志服务、导出服务、回放服务的边界还没有落地，和 `docs/serial_station_architecture.md` 的 services 目标结构不一致。

本阶段先实现 `SerialLogService`，只处理结构化日志的记录、查询、格式化和内存容量控制，不直接接 UI，不操作串口线程，不写真实文件。

## 2. 目标

1. 在 `src/apps/serial_station/services/` 增加 `SerialLogService.h/.cpp`。
2. 提供结构化日志记录模型，覆盖 TX、RX、System、Error 四类方向。
3. 支持追加、清空、快照、过滤、最大容量裁剪。
4. 支持生成稳定的纯文本和 JSON Lines 文本，作为后续导出服务输入。
5. 增加 `tests/serial_station/test_serial_log_service.cpp`，不依赖真实 COM 口。

## 3. 非目标

- 本阶段不把 `SerialLogPanel` 改为直接使用 service。
- 本阶段不实现文件保存对话框或真实文件导出。
- 本阶段不修改协议解析、串口收发线程、Dispatcher 或 Registry。
- 本阶段不新增 UI 文案或 QSS。

## 4. 分层边界

本次改动属于 `services` 层。

- `services/SerialLogService` 可依赖 Qt Core 类型和轻量通用工具。
- 不 include `ui/`、`SerialStationWindow`、`QWidget`。
- 不 include `core/SerialManager`、`SerialPort` 或 worker。
- 不 include 具体协议目录。
- 后续 UI/Controller 接入必须通过 Controller 协调，UI 不直接跨层写文件。

## 5. 验收标准

- `SerialLogService` 能追加 TX/RX/System/Error 记录。
- 空文本和空 payload 记录被拒绝或规范化为不可污染日志的结果。
- 最大容量小于当前记录数时，旧记录被裁剪，新记录顺序稳定。
- 过滤支持方向、来源、关键字和时间范围。
- 纯文本输出包含时间、方向、来源、文本和十六进制 payload。
- JSON Lines 输出每条一行，字段稳定，payload 使用大写空格分隔 HEX。
- QTest 覆盖追加、过滤、容量裁剪、序列化和清空。
- 新增 `.h/.cpp` 加入 `CMakeLists.txt`。
- `EmbedDebug.bat` 启动探针仍通过。

## 6. 风险

- 如果直接让 UI 持有 service，会重新制造 UI 与服务状态耦合。本阶段避免接 UI。
- 如果 service 直接写文件，会提前侵入 `SerialExportService` 职责。本阶段只返回文本。
- 如果日志模型过度绑定协议事件，会导致协议层反向污染服务层。本阶段只记录事实字段。
