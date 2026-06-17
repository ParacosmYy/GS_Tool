# PRD-186 Serial Station 诊断日志回放方向保持

## 目标

保证 Serial Station 的 `System` 与 `Error` 诊断日志在导出为 JSON Lines 后再回放时仍保持原方向，避免诊断记录被还原为 RX，形成日志显示、筛选、统计、导出、回放一致的闭环。

## 范围

- `entry_from_event()` 从 payload 中恢复 `direction`。
- 仅接受已知方向 `tx/rx/system/error`，未知值按事件类型兼容回退。
- 增加 codec 单测覆盖 System/Error 回放方向保持。
- 增加 controller 级导出回放测试，覆盖持久化闭环。

## 非目标

- 不改变 JSON Lines 记录结构。
- 不改变日志导出、回放服务接口。
- 不新增协议、传输、脚本或打包入口。
- 不提升真实设备验证状态。

## 架构边界

- codec 只负责 controller log entry 与 protocol event 的转换。
- controller 继续通过 session operation 编排导出和回放。
- services 只处理文件读写，不理解 UI 筛选或统计语义。
- UI 不参与回放方向修复。

## 三轴状态

| 维度 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 单测、controller 持久化测试、全量 Python 门禁和启动 smoke 覆盖 |
| 用户 | `U3` | 用户回放导出的诊断日志时方向保持一致 |
| 设备 | `D2` | 维持替身/loopback 验证口径，真实硬件未补证 |

## 验收

```powershell
uv run pytest tests\python\unit\test_workbench_controller.py tests\python\unit\test_log_entry_codec.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
