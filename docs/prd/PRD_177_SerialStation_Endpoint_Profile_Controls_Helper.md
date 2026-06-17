# PRD-177 Serial Station Endpoint Profile Controls Helper

## 1. 目标

将 TCP/UDP Profile 端点回填规则从各自控件文件中抽离到独立 UI helper，减少重复逻辑，并让端点回填行为具备纯单测证据。

## 2. 范围

- 新增 `ui/endpoint_profile_controls.py`。
- 提供 `apply_endpoint_profile_controls()`。
- `tcp_controls.apply_tcp_profile_controls()` 与 `udp_controls.apply_udp_profile_controls()` 复用同一 helper。
- 补充单测覆盖匹配 mode、非匹配 mode 和无效端点。

## 3. 非目标

- 不改变 TCP/UDP 控件布局、默认 host/port、连接动作或 Profile 数据结构。
- 不改变 controller、transport、protocol 或 service 行为。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/endpoint_profile_controls.py` | 统一 TCP/UDP 端点 Profile 回填规则 |
| `ui/tcp_controls.py` / `ui/udp_controls.py` | 构建控件并传入各自编辑框和期望 mode |
| `ui/session_actions.py` | 读取 Profile transport 并触发控件回填 |
| `controllers/` / `core/` / `protocols/` | 不参与 QWidget 写入 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | TCP/UDP Profile 回填行为保持一致 |
| 设备状态 | `D2` | 维持已有 TCP/UDP 替身与 loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_endpoint_profile_controls.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
