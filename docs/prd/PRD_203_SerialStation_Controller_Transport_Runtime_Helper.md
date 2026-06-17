# PRD-203 Serial Station Controller Transport Runtime Helper

## 目标

将 Serial Station controller 中的 transport 运行态拆成独立 helper，集中管理当前 transport、transport registry、活跃连接模式和回调绑定，进一步降低主 controller 对连接生命周期细节的直接持有。

## 范围

- 新增 `controller_transport_state`，承接可用端口、可用模式、active local port、连接、断开和当前 transport 状态。
- `workbench_controller` 仅保留用户意图 facade 与服务编排，主文件门禁收紧到 250 行以内。
- 新增 transport runtime 单测，覆盖 fake 替换、回调绑定、endpoint profile 配置。

## 非目标

- 不新增 transport 类型。
- 不改变 TCP/UDP/UART 用户交互。
- 不改变 `EmbedDebug.bat` 或 uv script。
- 不提升真实硬件验证口径。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | transport runtime helper 单测、controller/registry 回归和全量门禁覆盖 |
| 用户 | `U3` | 用户入口和主流程不变，连接、发送、接收、日志、导出、回放仍通过 UI smoke |
| 设备 | `D2` | 替身与 loopback 验证维持，真实设备未验证 |

## 验收

- `uv run pytest tests\python\unit\test_controller_transport_state.py -q`
- `uv run pytest tests\python\unit\test_workbench_controller.py tests\python\unit\test_workbench_controller_connections.py tests\python\unit\test_transport_registry.py -q`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug --smoke`
- `cmd /c EmbedDebug.bat --smoke`

## 架构检查

- controller 主类不直接持有 transport registry。
- transport helper 不依赖 UI。
- driver 层仍只暴露 transport 抽象，不反向依赖 controller。
- services、protocols、core 依赖方向未改变。
