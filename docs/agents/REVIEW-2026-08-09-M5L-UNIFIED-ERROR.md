# M5l presentation-owned unified error 六角色记录

日期：2026-08-09  
范围：统一 ViewModel/MainWindow 错误状态；不修改 domain error contract、transport、recorder、
protocol worker、BLE 或 J-Link RTT。  
父代理唯一源码写入者；六角色只读复核；未创建/修改/运行测试专用资产。

## 六角色结果

| 角色 | 子代理 | 结果 | 关键结论 |
|---|---|---|---|
| 产品 | `019fe4f7-7c63-7790-8826-1c0b4a22347b` | pass with priority | 下一源码切片优先统一 local error；实时协议 timing/resync 和发布 provenance 是后续独立门。 |
| 架构 | `019fe4fd-61d3-7950-bf4c-e096ffeba23c` | revise | ViewModel 必须成为唯一 error owner；MainWindow 本地错误只能委托，异步 source gate 保持不变。 |
| UI 设计 | `019fe4f7-7ce9-71f0-9440-77671f85e9de` | revise | 当前双写错误栏和无来源状态容易混淆；本轮收敛为统一错误呈现，scope bar 另列。 |
| 开发 | `019fe4f7-7d25-7c21-a255-38cd00e85f1c` | revise | 当前证据不足以做真实 t1.5/t3.5；推荐不改 codec/transport，保留 UART timing 后置。 |
| 验证 | `019fe4f7-7d65-7f40-885e-f7f150f205cb` | revise | 无 COM 时不得宣称 UART/物理 timing；本切片可用 Qt offscreen、静态和包门验证。 |
| 打包/流程 | `019fe4f7-7da1-7002-b185-614c96944f75` | revise | provenance gate 仍缺版本资源、变体隔离、manifest/hash 和实际许可证材料，另开流程切片。 |

六个角色结果均已收敛并关闭；其中架构角色由 replacement reviewer 返回，未让超时代理继续占用运行资源。

## 独立复核与简化

- 旧路径有两份错误状态：ViewModel 的 `ErrorInfo/_error_message` 与 MainWindow 的直接 QLabel 写入；
- 本轮只修改 `presentation/viewmodels.py` 和 `presentation/main_window.py`：ViewModel 持有
  `ErrorInfo | None`，`show_local_error()` 复用 `CONFIGURATION`，MainWindow 只渲染；
- `error_changed` 传递 `ErrorInfo`/`None`，detail 只进入 tooltip；没有新增 error bus、错误队列、
  domain code、transport 分支、依赖或跨层 scope；
- session/replay 迟到事件过滤、关闭顺序、generation barrier、recorder/raw truth 和 RTT 边界未改；
- 单一错误槽的覆盖优先级仍是后续需求，不能用本轮切片冒充 session-scoped error aggregation。

## 验证记录

- Qt offscreen inline vector 输出 `M5l unified error vectors passed`：本地错误和结构化错误均经
  ViewModel 状态，错误栏/tooltip 可见，`clear_error()` 后状态为 `None` 且 UI 隐藏；
- `uv lock --check`、Ruff format/check、compileall、`scripts/check.ps1`：通过；
- `python -B` 动态导入当前包及 49 个子模块：共 50 个模块通过；
- M5l 当前源码重新构建 onedir/onefile，并分别启动、WM_CLOSE、退出；onedir launcher/GUI PID
  92132，onefile launcher PID 97076 / GUI PID 83208，最终残留进程数为 0；
- onedir `dist/SerialForge/SerialForge.exe`：3,056,455 bytes，SHA-256
  `319AB88FBA846073981EF71D26C37FD3BA304494C5D066663F1CDCA6C04D0DF7`；onefile
  `dist/SerialForge.exe`：47,548,769 bytes，SHA-256
  `757EF5832DA0CAC519B74E68587E8FD613BCA3A4116FE4F2737324520F40B8BA`；
- onefile archive 包含 `serialforge.domain.events`、`serialforge.domain.protocols`、
  `serialforge.domain.replay`；默认 onedir optional scan 为 0；PyInstaller warning 文件 43 行；
- 未创建测试文件、mock、fixture 或测试 harness；当前 `uart_port_count 0`，真实 UART/BLE/TCP/UDP/RTT
  硬件验收未运行。

## 未完成与后续门

- P0：授权 USB-UART COM 设备的 8N1/7E2/RTS-CTS、收发、拔插、吞吐、超时和 raw JSONL；
- G0：版本/PE metadata、default/BLE/onefile 产物隔离、manifest/hash、实际 NOTICE/许可证和 CI；
- P1：MAVLink stream resync 与 Modbus UART timing boundary，需单独设计并真实设备验证；
- M6：J-Link 驱动、目标板、RTT Telnet 和授权硬件最后验收。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
