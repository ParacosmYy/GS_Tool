# PRD-181 Serial Station 端点控件文案 Helper

## 目标

将 TCP/UDP 控件中重复散落的 placeholder、tooltip 和连接按钮文案集中到 `endpoint_control_text` helper，降低 UI 控件构建函数中的硬编码文案密度。

## 范围

- 新增 `python/embeddebug/serial_station/ui/endpoint_control_text.py`。
- 提供 `endpoint_control_text()`，按端点模式返回控件文案配置。
- `tcp_controls.py` 与 `udp_controls.py` 复用该 helper。
- 新增单测覆盖 TCP 与 UDP 两组文案规则。

## 非目标

- 不改变连接、发送、接收或 Profile 行为。
- 不改变端点默认 host/port 文本。
- 不改变 endpoint 校验、controller、transport 或服务层。
- 不新增脚本、打包入口或 legacy native 兼容路径。

## 架构边界

- helper 只属于 UI 层，输出用户可见文案配置。
- controls 继续负责 QWidget 创建、objectName、信号装配和翻译入口调用。
- controller、core、drivers 与 services 不依赖该 helper。

## 三轴状态

| 维度 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 单测、UI 架构测试、UI smoke 与全量 Python 门禁覆盖 |
| 用户 | `U3` | TCP/UDP 可见入口文案保持一致，用户路径不变 |
| 设备 | `D2` | 维持 TCP/UDP loopback 与替身验证口径，本轮不声明真实硬件提升 |

## 验收

```powershell
uv run pytest tests\python\unit\test_endpoint_control_text.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
