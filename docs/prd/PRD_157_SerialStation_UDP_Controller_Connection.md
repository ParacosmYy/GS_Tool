# PRD-157 Serial Station UDP Controller Connection

## 1. 目标

将已接入 registry 的 UDP datagram transport 纳入 `SerialWorkbenchController` 连接闭环，使上层可以通过稳定 API 建立 UDP endpoint、发送 payload，并把 endpoint 写入 Profile。

## 2. 范围

- 新增 controller 层 `connect_udp` / `connect_udp_result`。
- 复用 transport registry 创建 `udp` driver。
- 复用 `host:port` endpoint 配置格式。
- 抽出连接配置 helper，降低 controller 文件体积并复用 TCP/UDP endpoint 路径。
- 增加 registry 注入测试，覆盖 UDP 连接、发送和 Profile 持久化。

## 3. 非目标

- 本批不新增 UI 控件。
- 本批不声明真实硬件验证。
- 本批不改变默认启动入口和打包入口。

## 4. 架构边界

| 层 | 决策 |
|---|---|
| `controllers/` | 只编排 transport mode、endpoint 和 Profile 状态 |
| `drivers/` | 继续负责 UDP datagram 读写细节 |
| `ui/` | 本批不触碰，后续再接入可见入口 |
| `services/` | Profile 保存仍走既有 session operation |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | `tests/python/unit/test_transport_registry.py` |
| 用户状态 | `U2` | controller API 可用，UI 入口待后续批次 |
| 设备状态 | `D2` | Fake transport 替身注入验证 endpoint 与发送闭环 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_transport_registry.py tests\python\unit\test_workbench_controller.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```

## 7. 风险

- UDP UI 入口未接入前，用户仍不能从界面直接选择 UDP。
- UDP datagram 的真实网络环境差异仍需后续虚拟网络或真实设备证据补齐。
