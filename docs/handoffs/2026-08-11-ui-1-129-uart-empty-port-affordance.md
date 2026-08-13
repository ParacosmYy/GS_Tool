# UI-1.129 UART 空端口输入引导交接

日期：2026-08-11  
父代理：Codex   
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`  
源码切片：`local-ui-1.129`

## 交付

无 UART 端口时，`src/serialforge/presentation/controllers/connection_builder.py` 的可编辑
端口 combo 现在同时为 `QComboBox` 和内部 `QLineEdit` 设置同一 placeholder：
`未发现端口 · 点击刷新或输入 COMx`。端口仍是唯一允许手动输入的 selector，placeholder 不会
进入 `currentText()`、endpoint 或连接动作。

## 验证

- 真实组合根：星轨霓虹、月影深海、樱雾夜航 × 980×680/1180×780；placeholder 双层值一致；
  `COM99` 手动输入、清空和 currentText gate 兼容。
- 六个组合均 `horizontal maximum=0`，截图逐像素 `exact-white=0`。
- 视觉证据：[ui_review_ui129_uart_empty_port.png](../../build/ui_review_ui129_uart_empty_port.png)。
- `scripts/check.ps1`：pass；`python -m compileall -q src`：pass；`uv run ruff check src`：pass。
- 未创建、修改或运行单元测试、mock、fixture、test harness；未操作真实硬件。

## 架构与质量记录

`_port_combo` 的构造与 placeholder 仍由 `connection_builder.py` 管理；
`connection.py`/`connection_runtime.py` 继续只读取现有 `currentText()`，没有把 presentation
提示扩散到 domain/application/infrastructure。没有新增 timer、状态源、线程、I/O、依赖或 OTA/
AES/RTT/J-Link 行为。

首轮运行时验证暴露 Qt editable combo 不会自动将 combo placeholder 传播到内部 line edit，已在
同一 owner 内修正。架构师线程 `019fed79-c377-7823-a89e-c1c3127cf77d`、
`019fed7b-a8ca-7540-b9d7-68257c3985f5`、`019fed7d-f207-7342-9cd5-a9f00a3ca086` 均超时，
未形成独立通过结论；父代理完成 correctness/readability/architecture/security/performance 与
行为保持简化评估。对应决策见 [ADR-0116](../adr/0116-uart-empty-port-affordance.md)。

本轮未修改嵌入式 C/C++；`embedded-enterprise-workflow` 与
`embedded-code-review-simplifier` 对 Python/PySide6 presentation 切片不适用，public vendor
applicability=N/A；真实 UART、BLE、RTT/J-Link、EXE 启动和正式发行验收未运行。

## 打包

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
大小：47,947,205 bytes  
SHA-256：`7E6EA48736EDF112EEDEBA86B65BCBE1BF4C9DF8522AACEF6D0C61E140272620`  
archive listing SHA-256：`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`  
provenance：pass；签名：`NotSigned`；正式发行资格：`false`；硬件验收：`not_run`

根目录 `SerialForge.exe` 仍被 PID 46108、49236 的旧实例占用，未强制结束；根目录旧包仍为
47,931,714 bytes，SHA-256 `971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`
（请以 `docs/handoffs/current.md` 的校正值为准）。用户关闭旧实例后，再将 canonical 文件复制
到根目录并复核 SHA-256。
