# ADR 0002：采用 Windows Python + PySide6 + PyInstaller

- 状态：Accepted for MVP
- 日期：2026-08-09
- 取代：ADR 0001 的 Rust/Tauri 方案（保留作调研记录）

## 背景

项目目标是尽快得到一个可用的 Windows 嵌入式调试工具，首批需要 UART、文本/Hex 终端、日志和后续 TCP/UDP、BLE、RTT 扩展。Rust/Tauri 的长期结构很好，但会增加 Rust、MSVC、WebView2 和前端工具链的首次配置成本。

## 决策

首版采用：

- Windows 原生 Python 3.12；
- PySide6 负责界面；
- pyserial 负责 UART；
- bleak 作为可选 extra 负责 BLE GATT；
- Python 标准库 socket/asyncio 负责 TCP/UDP；
- PyInstaller 负责 EXE；
- RTT 首版通过外部 CLI/工具适配，暂不直接绑定 J-Link SDK/DLL。

代码仍必须采用 domain/application/infrastructure/presentation 分层，设备库通过端口接入，不能因为 Python 快速就把业务逻辑堆进 Qt 槽函数。

## 取舍

优点：不需要 WSL、Rust、MinGW、Visual Studio C++ 工具链；PySide6 自带 Qt Python 包；能快速完成 UART vertical slice。

代价：Python 高吞吐日志需要 worker、队列和批量刷新；BLE asyncio 与 Qt 线程需要边界；J-Link RTT 直接 SDK 集成更适合后续独立适配器；PySide6/Qt 许可证必须随发行策略确认。

## 通过条件

1. UART 在 Windows 上完成连接、收发、拔插、关闭和 EXE 启动验证；
2. UI 不直接依赖 pyserial，替换为内存/回环 Transport 时无需改 UI；
3. `docs/DEPENDENCIES.md` 与 PyInstaller 发行物包含许可证记录；
4. TCP/UDP、BLE、RTT 只能通过新增 infrastructure adapter 接入；
5. 高速输入有界、可观测，不允许无限增长的 Qt 文本控件或队列。
