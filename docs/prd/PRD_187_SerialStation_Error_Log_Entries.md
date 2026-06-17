# PRD-187 Serial Station 错误日志入库

## 目标

将 Serial Station controller 捕获到的连接、发送、注入等错误写入 `Error` 日志 entries，使错误可在 UI 日志区筛选、统计、导出和回放，而不是只显示在状态栏。

## 范围

- `_handle_error()` 追加 `error` 方向日志 entry。
- 保持既有错误回调，状态栏仍能显示错误。
- 单测覆盖发送前未连接错误进入 entries 与 log callback。
- UI smoke 覆盖发送失败后日志区出现 Error 行并更新 Error 统计。

## 非目标

- 不改变错误码命名。
- 不新增后台线程或服务接口。
- 不改变日志 JSON Lines 格式。
- 不提升真实设备验证状态。

## 架构边界

- controller 负责把 transport/controller 错误转为日志事实。
- UI 只通过既有 log callback 展示，不直接构造错误日志。
- services 继续只负责文件读写和回放。
- protocols、core、drivers 不依赖 UI 日志格式。

## 三轴状态

| 维度 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 单测、UI smoke、全量 Python 门禁和启动 smoke 覆盖 |
| 用户 | `U3` | 用户可在日志区筛选、统计并持久化错误诊断记录 |
| 设备 | `D2` | 维持替身/loopback 验证口径，真实硬件未补证 |

## 验收

```powershell
uv run pytest tests\python\unit\test_workbench_controller.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
