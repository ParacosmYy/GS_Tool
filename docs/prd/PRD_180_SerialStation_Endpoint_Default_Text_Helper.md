# PRD-180 Serial Station 端点默认文本 Helper

## 目标

将 TCP/UDP 控件中重复的默认端点文本集中到 `endpoint_default_text` helper，避免两个控件各自硬编码默认 host/port。

## 范围

- 新增 `python/embeddebug/serial_station/ui/endpoint_default_text.py`。
- 提供 `apply_default_endpoint_text()`，统一写入默认 host 与 port。
- `tcp_controls.py` 与 `udp_controls.py` 复用该 helper。
- 新增单测覆盖默认 host/port 文本规则。

## 非目标

- 不改变 TCP/UDP 连接行为。
- 不改变 endpoint 校验、Profile 持久化、controller 或 transport。
- 不新增脚本、打包入口或 legacy native 兼容路径。

## 架构边界

- helper 仅属于 UI 层，职责是设置控件默认文本。
- controls 继续只负责控件构建与信号装配。
- controller/core/drivers 不读取 QWidget，也不依赖该 helper。

## 三轴状态

| 维度 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 单测、UI 架构测试、UI smoke 与全量 Python 门禁覆盖 |
| 用户 | `U3` | 用户仍通过 TCP/UDP 可见入口获得相同默认端点体验 |
| 设备 | `D2` | 维持 TCP/UDP loopback 与替身验证口径，本轮不声明真实硬件提升 |

## 验收

```powershell
uv run pytest tests\python\unit\test_endpoint_default_text.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
