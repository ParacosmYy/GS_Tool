# PRD-169 Serial Station Serial Port Options Helper

## 1. 目标

将串口端口 Combo 的空态文案、选项刷新和有效性判断从 `connection_actions.py` 中抽离到独立 UI helper，避免连接动作层重复维护 `No serial ports` 语义。

## 2. 范围

- 新增 `ui/serial_port_options.py`。
- 提供 `populate_serial_port_options()`、`combo_has_serial_ports()` 和 `empty_serial_port_text()`。
- `connection_actions.py` 复用 helper 处理串口端口列表刷新和有效端口判断。
- 补充纯单测覆盖空态、当前端口保留和有效性判断。

## 3. 非目标

- 不改变串口枚举来源、controller、driver 或 transport 行为。
- 不新增真实串口硬件验证能力。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/serial_port_options.py` | 统一串口端口 Combo 空态、选项刷新和有效性判断 |
| `ui/connection_actions.py` | 编排连接动作并委托端口选项 helper |
| `controllers/` / `drivers/` | 提供可用端口列表和连接能力，不写 UI 空态文案 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 串口端口空态可见行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_serial_port_options.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
