# UI-1.130 UART 参数摘要 rail 几何交接

日期：2026-08-11  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`  
源码切片：`local-ui-1.130`

## 交付

`src/serialforge/presentation/uart_timing_surface.py` 的 `UartTimingSummarySurface` 仍负责
呈现当前 UART 波特率、数据位、校验、停止位和流控摘要，但纵向 `QSizePolicy` 从 `Preferred`
改为 `Fixed`，避免父级 connection grid 在专注模式把一行摘要拉伸成大块空面板。横向仍为
`Ignored`，以适配 980/1180 宽度。

## 验证

- 星轨霓虹、月影深海、樱雾夜航 × 980×680/1180×780；摘要高度均为 38px，size hint=38px。
- 所有组合当前页 `horizontal maximum=0`，截图逐像素 `exact-white=0`。
- 视觉证据：[ui_review_ui130_uart_summary.png](../../build/ui_review_ui130_uart_summary.png)。
- `scripts/check.ps1`：pass；`python -m compileall -q src`：pass；`uv run ruff check src`：pass。
- 未创建、修改或运行单元测试、mock、fixture、test harness；未操作真实硬件。

## 架构与质量记录

本轮只修改摘要组件的 presentation size policy；builder、connection controller、ViewModel、
TransportConfig、主题 token、selector signal、tooltip/accessibility 和连接行为保持不变。
没有新增 timer、状态源、动画、线程、I/O、依赖或 OTA/AES/RTT/J-Link 行为。

架构师线程 `019fed87-2dc8-7160-9148-192ecd61726f` 超时，未形成独立通过结论；父代理完成
correctness/readability/architecture/security/performance 与行为保持简化评估。对应决策见
[ADR-0117](../adr/0117-uart-timing-summary-geometry.md)。

本轮未修改嵌入式 C/C++；`embedded-enterprise-workflow` 与
`embedded-code-review-simplifier` 对 Python/PySide6 presentation 切片不适用，public vendor
applicability=N/A；真实 UART、BLE、RTT/J-Link、EXE 启动和正式发行验收未运行。

## 打包

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
大小：47,947,485 bytes  
SHA-256：`3AC144396C9EB88AD9B447572A3B01965E5EF07641A95621AEEEF2E2210DF52C`  
archive listing SHA-256：`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`  
provenance：pass；签名：`NotSigned`；正式发行资格：`false`；硬件验收：`not_run`

根目录 `SerialForge.exe` 仍被 PID 46108、49236 的旧实例占用，未强制结束；根目录旧包仍为
47,931,714 bytes，SHA-256 `971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`。
用户关闭旧实例后，再将 canonical 文件复制到根目录并复核 SHA-256。
