# UI-1.133 扩展工具站接入路线 glyph 交接

日期：2026-08-11  
父代理：Codex  共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 交付

扩展 / 工具站接入概览增加 `EmbeddedStationRouteGlyph`：96×64、资源无关、NoFocus、
鼠标透明、空 accessibility，绘制 OTA / 安全 / 调试方向的三节点接入路线。它只消费
父链 `ThemeSpec` 和共享 `MotionController` frame，不读取 capability policy、后端状态、
密钥或设备数据。

`EmbeddedStationOverview` 负责 glyph 的 `set_frame()`/`stop()` 静态回退；
`ExtensionPanelWidgets` frozen bundle 显式暴露 layout 与 overview；`workspace.py` 负责
组合引用，`lifecycle.py` 复用既有 `_motion_surfaces()`。没有新增 QTimer、线程、I/O、
动作、依赖或 OTA/RTT/J-Link 行为，`ExtensionStationSummary`、7 张能力卡、只读规划层
和 contract-only/attach-only 语义保持不变。

## 验证

- 三主题 × 980×680/1180×780 真实组合根通过。
- overview 宽度为 908/1108px、高度 101px；route geometry 为 96×64。
- extension scroll horizontal maximum=0。
- 共享 frame animated/stop、隐藏窗口静态回退通过。
- 视觉证据：[build/ui_review_ui133_extension.png](../../build/ui_review_ui133_extension.png)。
- `scripts/check.ps1`、compileall、Ruff、源码行数门禁、onefile provenance 均通过。
- 未新增或运行单元测试资产，未启动真实 EXE、未操作硬件或 vendor 工具。

## 架构与审查

架构师线程 `019fed9e-6e2f-7722-9a21-b53cebea67ac` 在两次等待窗口内超时；独立审查线程
`019feda2-220e-7e13-b4da-7067f2f32bfa` 同样超时，均未计为独立通过。父代理完成
correctness/readability/architecture/accessibility/performance/rollback 六轴审查与行为保持、
简化评估。ADR 为 [ADR-0120](../adr/0120-extension-station-route-glyph.md)。未修改嵌入式
C/C++，public vendor applicability=N/A，真实硬件验证 `not_run`。

## 产物

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-ui-1.133`  
大小：47,951,506 bytes  
SHA-256：`1E2004935AFC936E9BD569C0063552EFC57DEE2C0F20B6DE64A494CB4D0F7BE6`  
archive listing SHA-256：`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`  
签名：`NotSigned`；`release_eligible=false`；硬件验收：`not_run`。

根目录旧 `SerialForge.exe` 仍被 PID 46108、49236 占用，未强制终止进程；根目录覆盖状态
仍为 `pending-user-close`，canonical 产物是本轮最新包。
