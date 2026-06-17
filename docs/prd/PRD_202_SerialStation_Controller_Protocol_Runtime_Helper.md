# PRD-202 Serial Station Controller Protocol Runtime Helper

## 目标

将 Serial Station controller 中的协议运行态拆为独立 helper，集中管理协议 registry、dispatcher、接收状态和日志化协议切换，减少主 controller 对协议运行细节的直接持有。

## 范围

- 新增 `controller_protocol_state`，承接协议列表、协议切换、接收字节转发和日志 entry 转事件。
- `workbench_controller` 继续保留对 UI 意图的编排，但不直接创建协议 registry 或 dispatcher。
- 拆分 `test_workbench_controller.py` 中的连接/Profile 场景，降低单测试文件职责密度。

## 非目标

- 不新增协议类型。
- 不改变 RawData / FireWater / JustFloat 行为。
- 不改变 UI 文案或启动链路。
- 不提升真实硬件验证口径。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 协议运行态 helper 单测、controller 连接回归和全量门禁覆盖 |
| 用户 | `U3` | 用户路径不变，协议切换、发送、接收、导出、回放仍走既有 UI smoke |
| 设备 | `D2` | 仍基于替身与 loopback 验证，真实设备未验证 |

## 验收

- `uv run pytest tests\python\unit\test_controller_protocol_state.py -q`
- `uv run pytest tests\python\unit\test_workbench_controller.py tests\python\unit\test_workbench_controller_connections.py tests\python\unit\test_transport_registry.py -q`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug --smoke`
- `cmd /c EmbedDebug.bat --smoke`

## 架构检查

- controller 主类不直接创建协议 registry。
- protocol runtime helper 不依赖 UI。
- receive state 仍保持 controller 层内部 helper，不上移到 core。
- services、drivers、protocols 依赖方向未改变。
