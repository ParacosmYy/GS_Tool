# PRD-143 Serial Station TCP Action Boundary

## 目标

将 Serial Station TCP 连接动作归并到 `ui/connection_actions.py`，让 `tcp_controls.py` 只负责控件构建和 Profile 控件回填，减少 UI 工具模块的职责混杂。

## 范围

- TCP host/port 校验与 `connect_tcp_result()` 调用归入 connection action。
- `MainWindow._connect_tcp()` 统一委托 `connection_actions.connect_tcp()`。
- 新增 UI 架构测试，防止 TCP 连接动作回流到控件构建模块。
- 既有 TCP 连接成功、失败和 Profile 保存 UI smoke 保持不变。

## 非目标

- 不改变 TCP transport、Profile 格式、连接错误码和用户可见文案。
- 不新增脚本入口，不改变 `EmbedDebug.bat -> uv run start-embeddebug`。
- 不提升真实设备验证状态。

## 验收

- `tcp_controls.py` 不再调用 controller，不直接写状态栏。
- `connection_actions.py` 持有 Fake/Serial/TCP/Send 的用户动作。
- TCP endpoint 成功连接与失败提示 UI smoke 继续通过。
- `uv run test-embeddebug-py`、`uv run test-embeddebug-tools` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，UI action 边界有架构测试与 pytest-qt 覆盖。
- 用户状态：`U3`，TCP 用户路径保持稳定。
- 设备状态：`D2`，本轮延续 TCP loopback 替身验证口径，不新增真实硬件证据。
