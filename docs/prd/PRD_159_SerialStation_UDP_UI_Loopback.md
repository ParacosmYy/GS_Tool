# PRD-159 Serial Station UDP UI Loopback

## 1. 目标

将 UDP 从“可见连接入口”推进到“PyQt UI 可验证收发闭环”。用户通过界面连接 UDP endpoint 后，可以发送 datagram 到对端，并接收对端回包进入日志。

## 2. 范围

- 增加 UDP UI loopback smoke，使用真实 `QUdpSocket` peer 验证 TX/RX。
- UDP 连接成功状态显示本地绑定端口，方便对端回包。
- Controller 只暴露只读诊断端口，不让 UI 直接访问 driver。
- README 与评分追踪同步到本轮证据。

## 3. 非目标

- 不新增 UDP 监听配置面板。
- 不做组播、广播、多端点路由。
- 不声明真实设备或跨主机网络验证完成。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/connection_actions.py` | 展示连接结果与本地端口诊断 |
| `controllers/workbench_controller.py` | 提供当前 transport 本地端口只读诊断 |
| `drivers/udp_datagram.py` | 继续负责 datagram 收发 |
| `tests/python/ui_smoke/` | 通过 Qt UDP peer 验证 UI 收发闭环 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | `uv run test-embeddebug-py` |
| 用户状态 | `U3` | UI 可连接、发送、接收、记录日志 |
| 设备状态 | `D2` | 本机 `QUdpSocket` peer 替身验证 |

## 6. 验收

```powershell
uv run pytest tests\python\ui_smoke\test_serial_station_udp_ui.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
