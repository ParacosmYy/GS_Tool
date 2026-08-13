# UI-1.134 UART selector 响应式宽度交接

日期：2026-08-11  
父代理：Codex  共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 交付

修复 980×680 链路 / 连接页的 UART 八列 grid 横向溢出。根因是 port combo 240px 上限、
长流控 option 约 218px size hint 与多列间距叠加，使 UART layout minimum=1013px；可用
内宽为 908px。`connection_builder.py` 继续使用 `_configure_bounded_combo()`：port 上限
调整为 200px，data/parity/stop/flow 分别为 84/110、84/150、84/130、110/160px，
baud 沿用 110/150px。

端口仍支持手输 COMx，其他 selector 仍不可编辑；itemData、选项、defaults、signals、
tooltip/accessibility、焦点顺序、连接 gate 和 transport DTO 均保持不变。闭合 combo 的
长标签允许 Qt elide，popup 与辅助描述保留完整选择语义。没有新增状态、resize 状态机、
timer、线程、I/O、依赖或 OTA/debug 行为。

## 验证

- 三主题 × 980×680/1180×780 × UART/TCP Client/TCP Server/UDP/BLE GATT/J-Link RTT。
- 四个 workspace tab 一并覆盖，共 60 组；全部 `hmax=0`、`exact-white=0`。
- UART layout minimum=872px，980px 可用内宽 908px；视觉证据：
  [build/ui_review_ui134_connection_responsive.png](../../build/ui_review_ui134_connection_responsive.png)。
- `scripts/check.ps1`、compileall、Ruff、源码行数门禁、onefile provenance 均通过。
- 未新增或运行单元测试资产，未启动真实 EXE、未接入硬件或 vendor 工具。

## 架构与审查

架构师线程 `019feda9-d39f-7d11-9433-1a6c34ea1ef3` 已调用但在两次等待窗口内超时，未计为
独立通过；父代理完成 owner、响应式、可读性、可访问性、行为保持与简化复核。ADR 为
[ADR-0121](../adr/0121-uart-selector-responsive-width.md)。未修改嵌入式 C/C++，public
vendor applicability=N/A，真实硬件验证 `not_run`。

## 产物

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-ui-1.134`  
大小：47,950,558 bytes  
SHA-256：`A5282CDCFF9FF31321C46DD382E8A04CA1BCE39BE16B05D6A3CC4004C0B91057`  
archive listing SHA-256：`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`  
签名：`NotSigned`；`release_eligible=false`；硬件验收：`not_run`。

根目录旧 `SerialForge.exe` 仍被 PID 46108、49236 占用，未强制终止进程；根目录覆盖状态
仍为 `pending-user-close`，canonical 产物是本轮最新包。
