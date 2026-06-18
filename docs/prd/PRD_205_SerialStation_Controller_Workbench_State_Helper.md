# PRD-205 Serial Station Controller Workbench State Helper

## 目标

将 Serial Station controller 中的日志 entries 和命令历史两组共享可变状态拆成独立 helper，减少主 controller 对 list 生命周期的直接持有，让状态快照、清空和后续演进有稳定落点。

## 范围

- 新增 `controller_workbench_state`，统一承接 entries 与 command history。
- `workbench_controller` 通过 workbench state 访问日志和命令历史，不再直接创建两组 list。
- 新增 workbench state 单测，覆盖只读快照和清空日志不影响命令历史。

## 非目标

- 不改变 UI 交互和用户入口。
- 不改变日志、回放、Profile 的文件格式。
- 不新增协议、transport 或设备能力。
- 不提升真实设备验证口径。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | workbench state helper 单测、controller 回归和全量门禁覆盖 |
| 用户 | `U3` | 用户入口和主流程不变，状态组织变化对用户透明 |
| 设备 | `D2` | 替身与 loopback 验证维持，真实设备未验证 |

## 验收

- `uv run pytest tests\python\unit\test_controller_workbench_state.py -q`
- `uv run pytest tests\python\unit\test_workbench_controller.py tests\python\unit\test_workbench_controller_connections.py tests\python\unit\test_workbench_controller_profiles.py -q`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug --smoke`
- `cmd /c EmbedDebug.bat --smoke`

## 架构检查

- controller 主类不直接创建 entries 与 command history list。
- state helper 不依赖 UI、drivers、protocols 或 services。
- 日志导出、回放、Profile 仍通过原 service/helper 路径。
- services、protocols、core 依赖方向未改变。
