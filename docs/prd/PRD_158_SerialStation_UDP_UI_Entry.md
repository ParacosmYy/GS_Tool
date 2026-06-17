# PRD-158 Serial Station UDP UI Entry

## 1. 目标

把 UDP datagram 连接从 controller API 推进到 PyQt 用户入口。用户可在 Serial Station 主界面填写 UDP host/port，点击 Connect UDP 后建立连接，并将 endpoint 保存到 Profile。

## 2. 范围

- 新增 UDP host、port、Connect UDP 控件。
- UI 入口只发意图，实际连接继续走 `connection_actions.connect_udp` 与 controller。
- Profile 加载时可回填 UDP endpoint。
- 增加 UI smoke 和 UI 架构边界测试。

## 3. 非目标

- 本批不做 UDP 服务端监听、多端点并发或组播配置。
- 本批不声明真实硬件验证。
- 本批不改变启动、测试、打包入口。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/udp_controls.py` | 构建控件、回填 Profile，不调用 controller |
| `ui/connection_actions.py` | 校验字段并调用 controller |
| `ui/main_window.py` | 只保留 `_connect_udp` 委托 |
| `controllers/` | 保持既有 UDP 连接 API |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | UI smoke + unit 架构测试 |
| 用户状态 | `U3` | 主界面可见 UDP 连接入口、状态反馈、Profile 保存 |
| 设备状态 | `D2` | PyQt smoke 使用替身 open 验证 endpoint |

## 6. 验收

```powershell
uv run pytest tests\python\ui_smoke\test_serial_station_udp_ui.py tests\python\unit\test_serial_station_ui_architecture.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
