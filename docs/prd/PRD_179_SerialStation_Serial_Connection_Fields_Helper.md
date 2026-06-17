# PRD-179 Serial Station Serial Connection Fields Helper

## 1. 目标

将串口连接入口中的 combo 字段读取与类型转换规则从 `connection_actions.py` 抽离到独立 UI helper，让连接动作只负责校验、调用 controller 和更新状态。

## 2. 范围

- 新增 `ui/serial_connection_fields.py`。
- 提供 `read_serial_connection_fields()` 与不可变 `SerialConnectionFields`。
- `connection_actions.connect_serial()` 复用 helper 读取 port、baud、data bits、parity、stop bits 和 flow control。
- 补充纯单测覆盖字段读取、整数转换和大小写规范化。

## 3. 非目标

- 不改变串口连接参数、controller API、transport 实现或 Profile 数据结构。
- 不改变 TCP/UDP 连接入口、端点校验、日志、导出或回放逻辑。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/serial_connection_fields.py` | 读取并规范化串口连接 UI 字段 |
| `ui/connection_actions.py` | 校验端口存在性、调用 controller、更新状态 |
| `controllers/` / `core/` / `drivers/` | 不读取 QWidget combo |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | 串口连接字段行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_serial_connection_fields.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
