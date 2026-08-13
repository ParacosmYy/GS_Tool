# ADR 0006：M3 BLE GATT 单设备异步会话

状态：已接受（2026-08-09；无硬件时只可条件交付）

## 背景

SerialForge 已有 UART、TCP Client/Server 和 UDP 的同步 worker 边界，但 BLE GATT 的
连接、WinRT 回调和 notification 依赖 asyncio。把 BLE 压成普通 `StreamWrite` 会丢失
characteristic、写入模式、MTU 和通知生命周期，也容易在 Qt 线程错误地关闭 Bleak 对象。

## 决策

- M3 只交付 Windows BLE Central 的单设备会话；扫描与连接分别使用独立的
  `BleGattDiscoveryPort` 和 `BleGattTransportPort`；
- `BleGattTransportConfig` 只跨层传递设备稳定 ID、名称、service UUID 过滤、connect timeout、
  `pair`、Windows service-cache 选项和 schema，不传递 `BLEDevice`、`BleakClient` 或 WinRT 对象；
- 每个 characteristic 用 `service_uuid + characteristic_uuid + handle` 作为 UI-safe 实例身份，
  避免同 UUID characteristic 歧义；服务/characteristic properties、MTU 和
  `max_write_without_response_size` 作为不可变能力快照暴露；
- 操作使用 typed command：read、write-with-response、write-without-response、notify enable/disable。
  写入按 characteristic properties 校验，不做自动分片、盲目重试或把写入成功解释为设备应用已处理；
- Bleak 仅在 `ble` optional extra 的 infrastructure 模块内延迟导入。主线组合根可以构造 factory，
  但 UART/TCP/UDP 启动和不带 `ble` extra 的包不得导入 `bleak`；
- `BleGattSessionManager` 独占一个 worker thread 和 asyncio event loop。Bleak client 创建、连接、
  服务发现、读写、start/stop notify、disconnect 与 loop 关闭均由该 worker 执行；Qt 只投递有界
  typed command，通知 callback 立即复制 `bytearray` 为 `bytes` 后发布 immutable event；
- 主动停止：停止接受命令 → 唤醒/取消当前 asyncio task → worker 停止 notification → disconnect →
  关闭 loop → 发布 `CLOSED`。设备/蓝牙/授权断线发布可恢复错误并结束本次会话，不自动重连或自动恢复订阅；
- 原始 JSONL 记录增加 `channel` 字段保存 characteristic 实例 key；BLE read/notification 属于 RX，
  GATT write completion 属于 TX。UI 复用已有 raw/Hex 预览，不把 Bleak 对象传到 Qt。

## 范围

- 手动开始/停止扫描；名称子串和 advertised service UUID 过滤；设备 ID、名称、RSSI、广播 service 显示；
- 单设备连接、服务/characteristic 发现、属性显示、read、两种 write、notify 开关、MTU/写长提示；
- 需要配对/加密/授权、服务变化、写入失败、通知失败、设备断线均给出可行动错误。

## 不在 M3

多设备并发、Bluetooth Classic SPP、descriptor/CCCD 编辑、自动重连/重订阅、可靠重放、自动分片、
OTA、任意 WinRT 操作、保存凭据、Linux/macOS 适配，以及真实设备未验证时的“实机通过”声明。

## 依据与验证边界

Bleak 官方 API 支持 scanner discovery、GATT service/characteristic read/write 和 notification；
其 Windows backend 基于 WinRT，且文档特别说明 WinRT apartment/event-loop 边界和 client 的
`mtu_size`、写入模式、notification API 需要按后端处理：

- <https://bleak.readthedocs.io/en/stable/api/scanner.html>
- <https://bleak.readthedocs.io/en/stable/api/client.html>
- <https://bleak.readthedocs.io/en/latest/backends/windows.html>

无硬件时允许验收：依赖锁定、缺依赖错误、配置边界、静态检查、启动/关闭、主线包不收集 Bleak/WinRT、
BLE extra 包能启动。扫描、服务发现、配对/重新授权、通知收发/停止、断线、MTU 和两种写入必须在
真实 BLE 设备上记录设备型号、Windows/蓝牙驱动、service/characteristic UUID、properties、MTU、
结果和未运行项。
