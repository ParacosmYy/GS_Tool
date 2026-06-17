# PRD-182 Serial Station 日志筛选选项 Helper

## 目标

将日志筛选下拉中的默认选项 `All/TX/RX` 从 `sections.py` 中抽出为 `log_filter_options` helper，为后续扩展 System/Error 等日志方向保留单点规则入口。

## 范围

- 新增 `python/embeddebug/serial_station/ui/log_filter_options.py`。
- 提供 `log_filter_options()` 与 `default_log_filter_text()`。
- `sections.py` 构建日志筛选下拉时复用该 helper。
- 新增单测覆盖默认选项和默认选中项。

## 非目标

- 不改变日志过滤算法。
- 不改变日志服务、controller、导出或回放行为。
- 不新增日志方向，不改变现有 UI 可见选项顺序。
- 不新增脚本、打包入口或 legacy native 兼容路径。

## 架构边界

- helper 只属于 UI 层，输出日志筛选控件的选项文本。
- `sections.py` 继续负责 QWidget 创建、objectName、翻译入口和信号装配。
- `log_actions` 与 `log_entry_filter` 的过滤行为保持不变。
- controller、services、core 与 drivers 不依赖该 helper。

## 三轴状态

| 维度 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 单测、UI 架构测试、UI smoke 与全量 Python 门禁覆盖 |
| 用户 | `U3` | 日志筛选入口选项保持一致，用户路径不变 |
| 设备 | `D2` | 维持 TCP/UDP loopback 与替身验证口径，本轮不声明真实硬件提升 |

## 验收

```powershell
uv run pytest tests\python\unit\test_log_filter_options.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
