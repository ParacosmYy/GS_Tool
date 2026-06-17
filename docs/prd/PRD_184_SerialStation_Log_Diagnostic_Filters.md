# PRD-184 Serial Station 诊断日志筛选

## 目标

让 Serial Station 日志筛选入口可直接查看 `System` 与 `Error` 方向日志，补齐连接、配置、Profile、异常恢复等诊断链路的基础可观测性。

## 范围

- `log_filter_options()` 增加 `System` 与 `Error` 选项。
- `log_entry_matches_filter()` 增加 System/Error 方向匹配规则。
- 搜索匹配使用真实日志方向作为前缀，避免非 TX 日志被归入 RX 搜索前缀。
- 补充单测覆盖诊断方向选项、方向过滤与搜索前缀。

## 非目标

- 不改变日志存储、导出、回放格式。
- 不新增后台线程、驱动或协议行为。
- 不改变 TX/RX/All 的既有筛选语义。
- 不新增脚本、打包入口或 legacy native 兼容路径。

## 架构边界

- UI 选项仍由 `log_filter_options` 提供，`sections.py` 只负责控件装配。
- 日志过滤规则仍集中在 `log_entry_filter`，`log_actions` 只传递当前 UI 文本。
- controller、core、protocols、services 与 drivers 不依赖 UI helper。
- 诊断能力先以可测试的纯函数规则固化，再由 UI 自动继承。

## 三轴状态

| 维度 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 纯函数单测、全量 Python 门禁和启动 smoke 覆盖 |
| 用户 | `U3` | 用户可从日志筛选下拉直接查看 System/Error 诊断日志 |
| 设备 | `D2` | 维持 TCP/UDP loopback 与替身验证口径，本轮不声明真实硬件提升 |

## 验收

```powershell
uv run pytest tests\python\unit\test_log_filter_options.py tests\python\unit\test_log_entry_filter.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
