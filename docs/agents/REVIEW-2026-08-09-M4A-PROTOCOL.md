# M4a 协议与组件基础切片 review（2026-08-09）

本轮只在 `SerialForge` 当前 checkout 工作，没有创建或使用 worktree。J-Link RTT 按用户要求继续排在最后；本轮没有安装、引入或打包 SEGGER/J-Link/probe-rs 后端。六角色协作完成后，父级负责唯一源码整合和最终验证。

## 六角色结论

| 角色 | 结果 | 冻结结论 |
|---|---|---|
| 产品 | 通过 | M4a 只覆盖 UART/TCP Client RX 的 Raw、Line、Delimiter、Length prefix、固定 checksum 和 bounded component preview；UDP/TCP Server/BLE 保留 raw 路径。 |
| 架构 | 通过（整改后） | `domain.protocols` 保持纯 decoder contract；`application.protocols` 独占 parser worker；source key 明确包含 session、peer/peer_id、channel、direction；raw recorder 不依赖 parser。 |
| UI 设计 | 通过 | 协议面板与终端并列，配置、应用、重置、作用域、状态和有限帧预览可见；不改变现有发送/原始 JSONL 记录路径。 |
| 开发 | 完成 | 新增 `ProtocolConfig`、`StreamingFrameDecoder`、`ProtocolPipeline`、bounded `ProtocolPipelineWorker`、typed events/port，以及 ViewModel/MainWindow 接线。 |
| 验证 | 通过非硬件门 | Ruff、compileall、锁文件、inline protocol vectors、offscreen Qt、EXE 启动/WM_CLOSE 和进程清理均通过；真实设备与压力项未运行。 |
| 打包/流程 | 通过 | 默认主线包不含 Bleak/WinRT；`-Ble` 变体可显式构建；默认和 BLE-enabled onefile 均实际启动验证。 |

## 代码边界和行为

- framing：Raw、LF/CRLF Line、任意有界 delimiter、1/2/4-byte length prefix；
- checksum：XOR-8、CRC16 Modbus、CRC32/IEEE；失败帧保留可见状态、payload Hex 和原因，不静默丢失；
- stream partial state 按 `ProtocolSource(session, transport, peer/peer_id, channel, direction)` 隔离；datagram/GATT 边界在 pipeline 层结束 partial frame；
- `ProtocolPipelineWorker` 有 ingress 数量和字节双上限，配置/reset 使用 generation 丢弃旧队列；关闭使用有限 join；
- parser 预览事件属于可丢弃 preview，原始终端与 JSONL raw recorder 不由 parser 成功与否决定；
- 当前只把 UART/TCP Client 的 stream RX 送入 parser，UDP、TCP Server peer 和 BLE notification/read 仍走原始终端/记录路径；
- M4a 不做 profile 保存迁移、JSON/TLV/字段 codec、TX 自动编码、数据表、曲线、过滤、回放或导出。

## 独立复核与简化评估

独立只读复核曾按流程启动 Luna/max、因并发/生命周期风险升级 Terra/max，并再次启动窄范围 Luna/max；这些代理在收束窗口内没有返回审查文本，均已关闭，未把超时当作通过。父级随后按 `embedded-code-review-simplifier` 清单完成第二遍审查，重点检查 `ProtocolPipelineWorker.offer/configure/reset/finish/shutdown/_run`、`StreamingFrameDecoder.feed/finish`、事件 sink、source identity 和 UI 生命周期；未发现 P0/P1 阻断项。

行为保持的安全简化/复用结果：

1. `_publish` 从 `object + type: ignore` 收紧为已有 `SessionEvent` union，减少类型绕过，不改变事件行为；
2. `ProtocolSource.direction` 纳入不可变 source identity，并要求 ingress direction 一致，避免未来 TX/RX 共享 partial buffer；
3. 没有把 pipeline、worker、transport 或 Qt 槽函数合并为“大一统组件”，因为这会破坏高内聚/低耦合和独立关闭边界；
4. 没有为减少行数改动队列/锁/关闭顺序；真实吞吐、压力和设备回调时序仍需硬件环境验证。

## 嵌入式工作流记录

本 checkout 没有 C/C++、MCU/SoC、BSP/HAL/CMSIS、RTOS、`.ioc`、`west.yml` 或固件构建目标；本轮改动是 Python/PySide6 Windows 桌面代码。因此适用的 vendor profile、芯片 revision、board、SDK、RTOS、compiler、linker 和厂商硬件契约均为 N/A/unknown，不应用任何厂商制造要求，也不宣称 MISRA、ISO 26262、ASIL、ASPICE 或认证合规。已读取并按其边界执行 `mcu`、`embedded-enterprise-workflow`、`embedded-code-review-simplifier` 及 `vendor-public-sources.md`。

## 实际验证证据

- `uv sync --locked --extra dev`：通过；默认环境 `bleak` 不可发现；
- `.\scripts\check.ps1`：通过，compileall 与 Ruff 通过；`uv lock --check`：通过；
- inline protocol vectors：跨 chunk Line/CRLF、Delimiter 空帧、Length split/incomplete、XOR invalid/valid、CRC16 Modbus、source isolation、worker event、direction/configuration bounds：通过；未创建测试文件、mock 或 harness；
- `QT_QPA_PLATFORM=offscreen` + 实际 Qt event loop：协议面板组合、窗口启动/关闭、session/protocol/recorder shutdown：通过；
- BLE-enabled onefile：`.\scripts\package.ps1 -Mode onefile -Ble`，48,790,747 bytes，SHA-256 `50C90D4995879EDE51E475CB4F8ACE780DB472411B925B67E22A4308C588A0EA`；实际启动、WM_CLOSE 和进程清理通过；
- 默认 onedir：`dist/SerialForge/SerialForge.exe`，2,882,782 bytes，SHA-256 `86F3677B8718911349D81D3F3BAC3ECE3DB202C1848D8CC297627B02898A7F46`；实际启动/WM_CLOSE 通过；文件名扫描未发现 Bleak/WinRT/Bluetooth/SEGGER/J-Link/probe-rs；
- 默认 onefile：`dist/SerialForge.exe`，47,378,490 bytes，SHA-256 `AE3DFF0F1C0C1BFE8A648862D795C3873FFC7A89F2B13F42402D1983ACEF78F8`；实际启动/WM_CLOSE 通过；验证结束无残留 `SerialForge` 进程；
- 默认 PyInstaller warning 仅记录源码中的 delayed optional `bleak` import；没有收集 Bleak/WinRT 后端。`-Ble` 变体会显式收集这些模块；两个变体共用 `dist`，切换后必须重新验证。

## 未运行项目和残余风险

- 未接真实 USB-UART 回环，未验证实际 7E2/RTS-CTS、拔插和高速吞吐；
- 未接真实 TCP/UDP 网络、BLE adapter/device，未验证扫描、配对、GATT、通知、MTU、断线恢复；
- 未做长时间 parser queue pressure、极限吞吐、clean Windows、签名、安装器和第三方许可证法律复核；
- M4b profile/codec/可视化和 M6 J-Link RTT 尚未开始；J-Link 驱动下载完成后仍需单独评审授权、外部工具桥接和实机验收。

## Gate 结论

M4a 达到代码、静态、手工、offscreen 和发行启动的非硬件交付门；不把这些证据冒充真实设备或认证结论。后续先做 M4b，再按路线最后进入 J-Link RTT。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
