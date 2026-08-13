# UI-1.126 交接：响应式工作区 viewport 边界

日期：2026-08-11  
范围：修复最小窗口下工作区 tab 与固定 route strip 的几何重叠。

## 实现

`presentation/controllers/workspace.py` 保留 `QTabWidget` 最大高度 350px，但移除与父布局冲突
的 220px 硬最小高度。980×680 时，tab viewport 收缩到 route strip 之前的可用空间；连接、协议、
命令和扩展页的既有 `QScrollArea` 继续承载剩余纵向内容。没有新增 resize handler、QTimer、业务
状态、滚动策略、设备 I/O 或 OTA/AES/RTT/J-Link 依赖。

## 验证

- 三主题 × 980×680/1180×780 × 四个工作区：exact-white=0。
- 当前 tab horizontal scrollbar maximum=0。
- `tab_route_overlap=false`，`route_shell_overflow=false`。
- 980×680：`workspaceShell=144px`、`tabs=109px`、`routeStrip=31px`，几何闭合且不覆盖。
- 1180×780：`workspaceShell=196px`、`tabs=161px`、`routeStrip=31px`，原有布局保持。
- `scripts/check.ps1`、compileall、Ruff 通过；截图已人工查看。

架构师线程 `019fed57-c98f-7330-a61a-7a69ce726a67` 在限定窗口内超时，未计为独立通过；父代理
完成 presentation owner、Qt 几何、可滚动性、无障碍、性能与简化审查。未修改嵌入式 C/C++；
embedded applicability=N/A。

## Assurance gate

- Public vendor source applicability：N/A。本轮没有 MCU/SoC、BSP/HAL、RTOS、ISR/DMA、driver 或
  embedded C/C++ source edit；OTA/AES/RTT/J-Link 仍只是既有 contract-only/attach-only 产品边界，
  未被本轮 UI 代码接入，因此没有可声明的 manufacturer constraint，也没有 MISRA/ISO/认证结论。
- Independent review：只读审查角色 `019fed5e-5086-72c3-9ddb-7c4051d3fd31` 在 60 秒窗口内超时并已关闭，
  未计为通过；父代理随后按 correctness/readability/architecture/security/performance 五轴完成复核，
  未发现必须修复项。
- Simplification assessment：保留既有 `QScrollArea` 和 route strip，只移除一个与父布局冲突的硬最小高度；
  未引入 resize 状态、timer、抽象层、依赖或跨层 facade，行为/生命周期/焦点/无障碍边界保持。
- Authorized non-destructive validation：`scripts/check.ps1`、compileall、Ruff、provenance verify，
  以及真实组合根 offscreen 三主题 × 双尺寸 × 四 tab 几何/像素审计均通过。未运行真实 GUI/EXE 启动、
  读屏、HIDPI、串口/网络/BLE/RTT 硬件和正式签名验收，因为本轮没有授权这些外部操作；硬件验收仍为
  `not_run`，release eligibility 仍为 `false`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.126`；47,943,352 bytes；SHA-256
`5C5F093587686FF2A56C409CC3D1F9A84823ECF1F1322D6F83EB7D4692133563`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`；provenance pass。根目录覆盖
仍等待 PID 46108、49236 退出；当前根目录仍是旧包 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。
