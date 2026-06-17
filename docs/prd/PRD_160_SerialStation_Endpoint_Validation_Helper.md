# PRD-160 Serial Station Endpoint Validation Helper

## 1. 目标

将 TCP/UDP 连接入口中的 host/port 校验抽为 UI 层公共 helper，减少 action 层重复逻辑，为后续多源连接入口继续扩展提供稳定基础组件。

## 2. 范围

- 新增 `ui/endpoint_validation.py` 纯函数校验模块。
- TCP/UDP connection action 复用同一校验结果。
- 新增单测覆盖合法 endpoint、空 host、非法 port。
- 保持现有 TCP/UDP UI 行为和错误提示不变。

## 3. 非目标

- 不新增新连接类型。
- 不改变 controller、driver 或协议层行为。
- 不声明真实硬件状态提升。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/endpoint_validation.py` | 字段级输入校验，不触碰 controller |
| `ui/connection_actions.py` | 展示校验结果并调用 controller |
| `controllers/` | 不参与 UI 字段校验 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测与 UI smoke 通过 |
| 用户状态 | `U3` | TCP/UDP 原有错误提示和连接路径保持 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_endpoint_validation.py tests\python\ui_smoke\test_serial_station_udp_ui.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
