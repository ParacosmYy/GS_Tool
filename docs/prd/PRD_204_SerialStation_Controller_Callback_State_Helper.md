# PRD-204 Serial Station Controller Callback State Helper

## 目标

将 Serial Station controller 中的日志、错误、测量回调集合拆成独立 helper，减少主 controller 对内部 list 的直接持有，让回调注册、通知和错误入库有单独测试入口。

## 范围

- 新增 `controller_callback_state`，统一管理 log、error、measurement 三组回调。
- `workbench_controller` 只保留公开 facade，内部通过 callback state 传递回调集合。
- 新增 callback state 单测，覆盖注册和错误通知入库。

## 非目标

- 不改变 UI 交互和用户入口。
- 不新增协议、transport 或设备能力。
- 不改变打包、启动和 uv script。
- 不提升真实设备验证口径。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | callback helper 单测、controller 回归和全量门禁覆盖 |
| 用户 | `U3` | 用户入口和主流程不变，连接、发送、接收、日志、导出、回放仍由 smoke 覆盖 |
| 设备 | `D2` | 替身与 loopback 验证维持，真实设备未验证 |

## 验收

- `uv run pytest tests\python\unit\test_controller_callback_state.py -q`
- `uv run pytest tests\python\unit\test_workbench_controller.py tests\python\unit\test_workbench_controller_connections.py tests\python\unit\test_controller_receive_state.py -q`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug --smoke`
- `cmd /c EmbedDebug.bat --smoke`

## 架构检查

- controller 主类不直接持有三组回调 list。
- callback helper 不依赖 UI。
- 错误路径继续复用 receive state 入库，不重复实现日志逻辑。
- services、protocols、core 依赖方向未改变。
