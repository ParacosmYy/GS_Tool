# M5j pipeline generation barrier 六角色记录

日期：2026-08-09  
范围：protocol → component → Dataset 异步派生链的 freshness/generation barrier；不扩展协议、
transport、BLE 或 J-Link RTT。  
写入者：父代理；六角色只读审查，唯一源码写入者为父代理；未创建/修改/运行测试专用资产。

## 六角色结果

| 角色 | 子代理 | 结果 | 关键结论 |
|---|---|---|---|
| 产品 | `019fe4da-dd3a-75f2-991b-299210259a52` | pass with priority | 真实 USB-UART loopback 是 P0；generation barrier 是 P1 必须先修的可信度问题；不把网络 loopback 冒充 UART 实机。 |
| 架构 | `019fe4da-dd7b-78e0-ab3f-40aebd5f76cc` | conditional pass | generation 必须跨 event DTO 传播；Replay session segment 和实时边界另行处理；保持三段 worker。 |
| UI 设计 | `019fe4da-ddb8-77c1-bc7f-5ca18a5cee34` | revise | 旧事件会污染 UI；建议 generation-chain 过滤；local error、已分帧契约和 transport inactive 视图留后续。 |
| 开发 | `019fe4da-ddf4-74e3-8419-20b52eccf982` | pass with narrow scope | 最小集合为 events/ports、三个 worker 和 ViewModel；先冻结 parser generation 契约，不新增协议能力。 |
| 验证 | `019fe4da-de34-7da0-9917-9e3a2418a8da` | revise | 需要当前源码统一复跑 inline/manual、Qt、静态、onedir/onefile/BLE 变体门；硬件仍单独列未运行。 |
| 打包/流程 | `019fe4da-de6e-77a0-9d9c-bd37a171616f` | conditional pass | 无需新 runtime dependency；正式发行仍被许可证/NOTICE、PE metadata、变体隔离和 CI 覆盖阻断。 |

六个角色均已完成并关闭。当前 checkout 是 Python/PySide6 Windows 应用，无 C/C++、MCU、固件、
BSP/HAL、RTOS、厂商 SDK 或硬件改动；嵌入式厂商约束适用性为 N/A，未作任何嵌入式认证声明。

## 独立复核与简化

- 旧实现只在 worker 内部保存 generation，`ProtocolFramesDecodedEvent`、
  `ComponentFramesDecodedEvent` 和 `DatasetBatchEvent` 没有传播 generation；UI 仅按 source/profile/
  config 过滤，无法保护后端统计/样本；
- 本轮只修改 `domain/events.py`、`domain/ports.py`、`application/protocols.py`、
  `application/components.py`、`application/datasets.py` 和 `presentation/viewmodels.py`；
- 每段 worker 的输出事件携带本阶段 generation；component 携带 protocol generation，Dataset 携带
  component/protocol generation；upstream fence 在 worker 入队时拒绝旧事件，commit/publish 再校验
  本阶段 generation；
- ViewModel 在 configure/reset 后同步 generation，在 protocol/component/Dataset 消费分支要求完整
  generation chain；没有增加 transport 分支、协议 codec、全局锁、动态插件或 runtime dependency；
- live `StreamDataReceivedEvent.occurred_at` 现在传入 `ProtocolIngressUnit`，协议事件保留捕获时间；
  raw recorder/terminal/transport 事实路径未改变；
- Replay session segment、local error clear、MAVLink/Modbus stream boundary 和正式许可证保持独立
  未完成项，没有用 generation 字段掩盖它们。

## 手工验证

未创建测试文件、mock、fixture 或测试 harness；使用临时 inline event/bytes 和真实 worker：

- protocol raw ingress 产生 generation 0 的 `ProtocolFramesDecodedEvent`，`occurred_at` 保留为
  ingress 时间；configure 后 root generation 递增为 1；
- component reset 设置 protocol fence=1，旧 protocol event generation=0 的 offer 被拒绝；
- Dataset reset 设置 component/protocol fence，旧 component event 被拒绝；fresh generation chain
  可以继续 offer；
- Ruff format/check、compileall 和既有 `scripts/check.ps1` 在最终源码后通过；Qt offscreen 应用
  创建/配置/关闭 API 兼容性通过；未触碰真实设备。
- PyInstaller onedir/onefile 均完成 GUI 启动、WM_CLOSE、进程退出检查；最终包为：
  `dist/SerialForge/SerialForge.exe`，3,055,171 bytes，SHA-256
  `6634E1E640B8C875D405A13D40B32C4B1BC0653600BC5D9343D7703392E99B14`；
  `dist/SerialForge.exe`，47,551,092 bytes，SHA-256
  `62083D4BBF75374F0A260F1AEC3B64840962E798AA6E95D933281C79ECE9812D`。
- onefile archive 确认包含 `serialforge.domain.codecs`、`serialforge.domain.events`、
  `serialforge.domain.mavlink`；默认 onedir optional scan 未发现 `Bleak`、`WinRT`、
  `SEGGER`、`JLink` 或 `probe-rs`；退出后 `SerialForge` 残留进程数为 0。

## 未运行与后续门

- P0：真实 USB-UART loopback，需记录设备型号、驱动、COM、参数、文本/二进制 TX/RX、JSONL raw、
  关闭和无残留进程；当前环境未提供授权硬件，不能用 TCP loopback 替代；
- Replay：原始 session 变化时的 segment reset/partial buffer 隔离；
- 协议：Modbus t1.5/t3.5、MAVLink stream resync、完整 dialect/TX；
- UI：local error 状态统一清除、已分帧契约的显式 UI 门、切换 RTT/UDP/TCP Server 时清空或标记派生视图；
- 发行：BLE-enabled 包、许可证/NOTICE、PE version metadata、输出目录隔离、CI onefile/hash/manifest；
- 硬件：BLE 实机、J-Link/目标板/RTT Telnet、干净 Windows、防火墙、长时间压力和正式签名。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
