# M5e 六角色复核记录：有界声明式批量命令

日期：2026-08-09  
范围：固定步骤宏、文本/Hex/CRLF、有限延时、TCP Server Peer 绑定、BLE typed write、
停止/失败状态、发送历史复用、Qt 编辑器和 PyInstaller 交付。  
源码写入者：父代理；六角色子代理均为只读复核，无子代理直接修改当前 checkout。

## 六角色记录

| 角色 | 子代理 run | 结论 | 关键建议 |
|---|---|---|---|
| 产品 | `019fe484-ff4e-7bf3-a16e-d2cef7605539` | pass with conditions → 已整合 | 只做 1–32 步静态顺序；不做脚本、循环、ACK、重试、广播、RTT 批量；“完成”只能表示本地提交 |
| 架构 | `019fe484-ff87-7bf2-872f-a9c60f8028c5` | conditional pass → 已整合 | 新增独立 `CommandBatchPort`/application worker；不把 batch list 塞进 `SessionPort`；绑定 immutable Peer/characteristic 快照 |
| UI 设计 | `019fe484-ffc7-7ec0-9e6f-1b664668f9a6` | pass with conditions → 已整合 | 常驻紧凑控制条、按需编辑器、步骤结果表、可访问状态文案；执行期间禁止编辑/普通发送 |
| 开发 | `019fe485-0014-7dc0-8292-ac3916eb6d59` | pass → 已整合 | 标准库线程 + 可取消 Event.wait；先构造全部 typed write，再逐条调用既有 SessionPort；不重复 recorder |
| 验证 | `019fe485-004e-7d62-9da4-dff1ccba87d1` | pass conditionally → 已整合 | 覆盖 16/32/512 B/16 KiB、失败、停止、顺序、offscreen、关闭和双模式打包；真实设备仍待授权环境 |
| 打包/流程 | `019fe485-008c-7572-ac4d-6dc246af9412` | revise → 已整合 | 无新增依赖；默认包保持 BLE/J-Link vendor-free；onedir/onefile 均需真实启动关闭；许可证/签名/clean Windows 仍是发行门 |

六个子代理已关闭；父代理负责整合、最终源码检查和验证。

## 变更摘要

- `domain/commands.py` 新增 immutable `CommandBatch`、`CommandBatchStep`、`CommandBatchRequest` 和 `CommandBatchSnapshot`；
- 容量固定为最多 16 个宏、32 步/宏、512 B/步（含 CRLF）、16 KiB/宏、2,000 ms/步、180 s 总等待；
- `application/commands.py` 提供共享 `build_transport_write()`，单条发送和批量发送复用同一 UART/TCP/UDP/TCP Server/BLE/RTT typed write 构造路径；
- `CommandBatchService` 仅在 RTT 之外运行，先预构造全部写入，再由 daemon worker 逐条 `SessionPort.send()`；延时可被停止事件打断；
- TCP Server 必须使用当前显式 `PeerId`，BLE 必须使用当前 characteristic/write mode；不广播、不自动切换目标、不自动分片；
- 已接受步骤写入既有历史，真实 TX 仍由 session worker 进入 raw recorder；批量 worker 不重复记录；
- presentation 新增 `CommandBatchEditorDialog` 和发送面板的批量命令选择/编辑/执行/停止/步骤状态表；
- 回放期间禁用批量命令；窗口关闭先停止 batch，再由组合根关闭 session；宏定义只保留在当前进程内存。

## 重要语义

本轮没有新增 operation/correlation ID，`SessionPort.send()` 的成功语义是“写入本地有界出站队列”。
因此 UI 使用“已提交/已完成”文案，不宣称线缆已经发送、对端已经处理或 BLE/TCP 应用层 ACK 已到达。
停止只阻止尚未提交的步骤，已经入队的数据不能撤回；session 断开不会把旧 TCP Peer 转投到新 Peer。

## 验证证据

已运行：

- `uv lock --check`：通过；
- `uv run --locked ruff format --check --no-cache src`：通过，47 files；
- `uv run --locked ruff check --no-cache src`：通过；
- `uv run --locked python -m compileall -q src`：通过；
- `scripts/check.ps1`：通过；
- inline manual vectors：宏容量、CRLF 线缆 payload、顺序、取消期间延时停止、第二步失败停止、历史数量和超限拒绝：通过；
- Qt `QT_QPA_PLATFORM=offscreen`：编辑器文本/Hex/延时保存、batch catalog/选择、窗口关闭和全部 worker shutdown：通过；
  有已知 `QFontDatabase` 缺少 PySide6 字体目录提示，不影响退出；
- TCP localhost loopback：真实 `SessionManager` + `NetworkTransportFactory` 接收两步 `AA55`，顺序和 batch completion：通过；
- `scripts/package.ps1 -Mode onedir`：通过；实际 GUI startup/WM_CLOSE/exit，PID `106552`；
- `scripts/package.ps1 -Mode onefile`：通过；实际 bootstrap/GUI child startup/WM_CLOSE/exit，bootstrap `57848`、GUI `71428`；
- 默认 onedir 文件名扫描中的 `Bleak|WinRT|Bluetooth|SEGGER|JLink|probe-rs` 匹配数：`0`；
- 验证结束时没有残留由本轮启动的 `SerialForge` EXE 进程。

最终产物：

| 产物 | 大小 | SHA-256 |
|---|---:|---|
| `dist/SerialForge/SerialForge.exe` | 3,025,139 B | `C2CC678390F475A221C1B09A2772071168F11FE15AA9B85AD01DC3FE994C3A5F` |
| `dist/SerialForge.exe` | 47,519,652 B | `EC39E9DDC62730BCBA43CC72405965D8DEEF90B0863082B75CD58BE6DD9A11E8` |

## 未运行/未宣称

- 未运行真实 UART 长时间吞吐、USB 拔插、真实 LAN 压力、UDP 丢包/大 datagram、BLE 扫描/配对/MTU/通知/写入实机；
- 未运行 J-Link RTT 宏；RTT 单条桥接仍需要用户已安装并启动的 J-Link Telnet 服务和授权目标板，驱动/工具继续留到最后；
- 未运行 batch 逐步线缆 completion、应用层 ACK、响应匹配、跨重启宏持久化、导入/导出和多 Peer 广播；这些不属于 M5e；
- 未运行干净 Windows、DPI/字体覆盖、代码签名、版本资源和第三方许可证发行审查；
- 未创建或修改单元测试、mock、fixture、test harness 或其他 test-only asset；
- 当前任务没有嵌入式 C/C++、固件、MCU、BSP/HAL/RTOS 或硬件源代码变更，因此没有新的厂商固件要求适用性声明。

## 简化评估

通过一个 immutable batch DTO、一个 `CommandBatchPort` 和一个标准库 worker 复用已有 typed send/历史/raw recorder，
没有引入 scheduler、脚本解释器、动态导入、数据库、插件运行时或新的传输 worker。将“队列已接受”明确为唯一
本轮可证明的完成语义，避免为所有传输重复构造无法可靠关联的 ACK/completion 链；固定步骤、固定 Peer、固定容量
也为后续 operation ID、宏持久化或更强协议编排保留了独立扩展点。
