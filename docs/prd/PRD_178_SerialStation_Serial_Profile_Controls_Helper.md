# PRD-178 Serial Station Serial Profile Controls Helper

## 1. 目标

将串口 Profile 中 baud/data/parity/stop/flow 回填规则从 `session_actions.py` 抽离到独立 UI helper，降低 session action 的控件细节负担，并补齐纯单测证据。

## 2. 范围

- 新增 `ui/serial_profile_controls.py`。
- 提供 `apply_serial_profile_controls()`。
- `session_actions.apply_profile_controls()` 继续负责读取 Profile 和触发回填，但串口 transport 参数委托 helper。
- 补充单测覆盖完整串口参数和缺省参数。

## 3. 非目标

- 不改变 Profile JSON 字段、TCP/UDP 端点回填、连接动作或 controller 行为。
- 不改变串口 transport、协议解析、日志、导出或回放逻辑。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/serial_profile_controls.py` | 统一串口 transport 参数 combo 回填规则 |
| `ui/session_actions.py` | 读取 Profile、处理协议/端口和调用 helper |
| `controllers/` / `core/` / `protocols/` | 不参与 QWidget 写入 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 串口 Profile 参数回填行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_serial_profile_controls.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
