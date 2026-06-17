# PRD-185 Serial Station 诊断日志显示与统计

## 目标

让 Serial Station 日志区能真实显示并统计 `System` 与 `Error` 方向日志，避免诊断日志被渲染为 RX，提升连接、Profile、导出、回放和错误恢复路径的可观测性。

## 范围

- `append_log_entry_line()` 支持 TX/RX/System/Error 四类方向展示。
- `set_log_stats_label()` 增加 System/Error 计数。
- `log_actions.update_log_stats()` 从 controller entries 中统计四类方向。
- 补充单测与 UI smoke，覆盖空态、过滤态和诊断方向渲染。

## 非目标

- 不改变日志存储、导出或回放格式。
- 不新增日志方向枚举或后台线程。
- 不改变 System/Error 筛选入口顺序。
- 不新增脚本、打包入口或 legacy native 兼容路径。

## 架构边界

- `status_messages` 只负责 UI 文案格式化。
- `log_actions` 只负责 UI 层 entries 统计与展示调用。
- controller、services、core、protocols、drivers 不依赖 UI 统计格式。
- 本轮不把设备验证状态提升到 D3/D4。

## 三轴状态

| 维度 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 单测、UI smoke、全量 Python 门禁和启动 smoke 覆盖 |
| 用户 | `U3` | 日志区可直接识别 System/Error 诊断日志并查看计数 |
| 设备 | `D2` | 维持 TCP/UDP loopback 与替身验证口径，真实硬件未补证 |

## 验收

```powershell
uv run pytest tests\python\unit\test_status_messages.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
