# UI-1.131 网络与 RTT responsive hint 几何交接

日期：2026-08-11  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`  
源码切片：`local-ui-1.131`

## 交付

`src/serialforge/presentation/controllers/connection_builder.py` 的共享
`_configure_responsive_hint()` 保留 word-wrap、120/520px 宽度边界、tooltip、accessible
description 和横向 `Ignored`，把纵向 `QSizePolicy` 从 `Preferred` 改为 `Fixed`。TCP Client
与 RTT 说明不再把专注模式的剩余空间撑成空面板；TCP Server allowlist、UDP/BLE/UART 和连接
动作不变。

## 验证

- 三主题 × 980×680/1180×780 × UART/TCP Client/TCP Server/UDP/BLE GATT/J-Link RTT，共 36 组。
- 说明按 size hint 收敛；标题/active panel 无重叠；`horizontal maximum=0`；逐像素
  `exact-white=0`。
- 视觉证据：[TCP hint](../../build/ui_review_ui131_tcp_hint.png)、
  [RTT hint](../../build/ui_review_ui131_rtt_hint.png)。
- `scripts/check.ps1`：pass；`python -m compileall -q src`：pass；`uv run ruff check src`：pass。
- 未创建、修改或运行单元测试、mock、fixture、test harness；未操作真实硬件。

## 架构与质量记录

共享 helper 只做 presentation 几何与文本可发现性，不读取传输状态，不创建状态源，不改变
`TransportConfig`、连接 gate、ViewModel、session 或 infrastructure。没有新增 timer、动画、
线程、I/O、依赖或 OTA/AES/RTT/J-Link 业务行为。

架构师线程 `019fed8d-6fdc-7532-8fdd-387290c71924` 超时，未形成独立通过结论；父代理完成
correctness/readability/architecture/security/performance 与行为保持简化评估。对应决策见
[ADR-0118](../adr/0118-network-hint-geometry.md)。

本轮未修改嵌入式 C/C++；`embedded-enterprise-workflow` 与
`embedded-code-review-simplifier` 对 Python/PySide6 presentation 切片不适用，public vendor
applicability=N/A；真实 UART、BLE、RTT/J-Link、EXE 启动和正式发行验收未运行。

## 打包

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
大小：47,947,904 bytes  
SHA-256：`61379AF7545EBF6745B726D56BFA41121640055BCAEE92EA82B47351EA238AF2`  
archive listing SHA-256：`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`  
provenance：pass；签名：`NotSigned`；正式发行资格：`false`；硬件验收：`not_run`

根目录 `SerialForge.exe` 仍被 PID 46108、49236 的旧实例占用，未强制结束；根目录旧包仍为
47,931,714 bytes，SHA-256 `971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`。
用户关闭旧实例后，再将 canonical 文件复制到根目录并复核 SHA-256。
