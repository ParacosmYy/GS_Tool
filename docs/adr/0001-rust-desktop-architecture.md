# ADR 0001：采用 Rust 核心 + Tauri 2 桌面架构

- 状态：Superseded by ADR 0002 for MVP
- 日期：2026-08-09
- 范围：SerialForge Windows-first 嵌入式调试工作站

## 背景

产品需要同时处理 COM、BLE GATT、TCP/UDP 和 J-Link RTT，并持续接收高速日志。UI 还需要终端、Hex、包解析、曲线、记录和设备配置等多个视图。设备 I/O、并发关闭、错误传播和资源释放比普通表单应用更关键。

## 决策

采用：

- Rust 作为应用核心和所有设备 I/O 的实现语言；
- Tokio 作为异步运行时；
- Tauri 2 作为 Windows 桌面壳；
- 前端只通过显式 command/event DTO 调用后端；控制面使用 commands，高速数据面使用 Tauri Channel 或拉取式 ring buffer；
- 传输通过 `TransportProvider`/`TransportHandle` 接入，使用 `IngressUnit`/`EgressOperation` 保留 stream、datagram、GATT characteristic 和 RTT channel 语义；协议解析通过 `FrameDecoder` 接入；
- 首版只使用 `probe-rs` 直接后端接入受限 RTT，J-Link SDK/DLL 作为与 probe-rs 互斥、默认 feature-off 且受授权约束的可选能力。

## 选择理由

- Rust 的类型、所有权和异步生态适合管理多连接、取消和错误传播；
- Tauri 可利用 Windows WebView2，前端适合做多面板工具 UI，最终产物仍是原生 Rust EXE；
- Tokio 可将串口、网络和后台解析统一到可取消任务模型；
- Transport 与 Protocol 分层能够复用终端、记录和可视化，不把每个设备类型写成一套产品；
- 显式 ingress/egress 语义能避免把 BLE write mode、UDP peer、RTT channel 等关键状态压缩成 `send(bytes)`；
- `probe-rs` 比直接封装 J-Link DLL 更适合作为第一阶段的 Rust 路线，但具体芯片和 RTT 行为仍需真实硬件验证。

## 考虑过的替代方案

### Rust 原生 UI（egui/Slint/iced）

优点是减少 WebView/Node 前置条件，打包路径直接；缺点是复杂桌面工具的表格、文本编辑、布局和生态需要更多自建。可以在 Tauri 原型验证后重新评估，但当前不作为默认方案。

### Qt/C++ 或 Python/PyQt

Qt 的串口和蓝牙模块成熟，且已有 `QuillForge` 使用 PyQt；但这是一个独立产品，Python EXE、Qt/PyQt 授权、跨驱动异步模型和 J-Link Rust 生态组合不如 Rust 核心统一。不能因为工作区已有 PyQt 项目就强行复用其运行时。

### Electron

前端生态强，但随包运行时更重，原生设备模块和 Windows 驱动边界更复杂；在当前 Windows-first、设备 I/O 密集的目标下不优先。

## 后果

正面：

- 设备层有清晰的线程和资源所有权；
- UART、网络、BLE、RTT 可以共享记录和解析能力；
- 后续可用 Tauri 前端快速迭代交互；
- 记录器、UI 预览和命令发送可以拥有独立的背压与丢弃策略。

代价与风险：

- 初始开发需要 Rust、MSVC、WebView2 和前端工具链；
- BLE 的 Windows 兼容性和蓝牙经典 SPP 需要独立验证；
- J-Link SDK 的官方授权限制会影响闭源分发；
- probe-rs 的 Windows 驱动模式可能与官方 J-Link 工具共存性冲突，必须记录和验证；
- 高吞吐数据不能默认走 JSON 事件，前端 Channel/拉取式缓冲和记录器完整性需要单独验证；
- Tauri command/event DTO 需要版本化，不能把内部 Rust 类型直接暴露给 UI；
- 若未来需要完整调试器，必须单独处理芯片数据库、DAP/GDB、烧录和目标安全状态。

## 通过条件

开始源代码实现前必须：

1. 安装并锁定 Rust stable MSVC 与 Tauri 版本；
2. 确认第一版只做受限 RTT 终端，不承诺完整 J-Link 调试器；
3. 选定一个真实 USB-UART、一个 BLE GATT 设备和一个可授权的 J-Link 验证组合；
4. 生成依赖许可证清单，并在引入 J-Link SDK 前取得分发授权结论；
5. 明确 probe-rs 与 SEGGER SDK/DLL 的互斥后端和 Windows 驱动方案；
6. 先完成 UART vertical slice，再加其他 Transport；
7. 为自动重连、TCP Server、BLE 配对和解析器设置稳定身份与资源上限。
