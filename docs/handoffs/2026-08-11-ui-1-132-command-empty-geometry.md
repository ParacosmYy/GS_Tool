# UI-1.132 批量命令空态 copy 几何交接

日期：2026-08-11  
父代理：Codex  共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 交付

`presentation/command_batch_empty_state.py` 只调整 `_eyebrow`、`_title`、`_hint`
三个 QLabel 的纵向 size policy 为 `Fixed`。空态卡片继续使用 `Expanding/Preferred`，
glyph、CTA、共享 `MotionController`、`new_requested` signal、word-wrap、焦点与无障碍
语义保持不变；没有引入业务状态、timer、线程、I/O 或传输协议行为。

## 架构与审查

本轮 owner 仍是 `CommandBatchEmptyState`，没有把几何修复扩散到全局 QSS 或 shell。
ADR 为 [ADR-0119](../adr/0119-command-empty-state-copy-geometry.md)。架构师线程
`019fed95-29fe-7e73-b091-0acf973dbd97` 超时，未计为独立通过；父代理完成 owner、
响应式、可访问性、五轴质量与行为保持的简化复核。未修改嵌入式 C/C++，public vendor
applicability=N/A，真实硬件验证未运行。

## 验证

- 三主题 × 980×680/1180×780 真实 Qt 组合根通过。
- eyebrow/title/hint 实际高度为 14/19/30px；长 hint 无裁切。
- 当前页 horizontal maximum=0，exact-white=0。
- 视觉证据：[build/ui_review_ui132_command_empty.png](../../build/ui_review_ui132_command_empty.png)。
- `scripts/check.ps1`、`compileall`、Ruff、onefile provenance 均通过；未新增或运行单元测试资产，未操作硬件。

## 产物

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-ui-1.132`  
大小：47,945,280 bytes  
SHA-256：`68B7C0E6387C621FAAA889D8440DFCDB8F509DF407A01F833C36980BCC9254D2`  
archive listing SHA-256：`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`  
签名：`NotSigned`；`release_eligible=false`；硬件验收：`not_run`。

根目录旧 `SerialForge.exe` 仍被 PID 46108、49236 占用，未强制终止进程；因此根目录
覆盖状态仍为 `pending-user-close`，canonical 产物是本轮最新包。
