# PRD-201 Serial Station Controller Receive State Helper

## 目标

将 Serial Station controller 中的接收侧处理拆成独立 helper，避免主编排类继续承载 dispatcher feed、日志映射、测量批次缓存和错误入库细节。

## 范围

- 新增 `controller_receive_state`，负责接收字节后的事件映射、测量 ring 状态和错误日志回调。
- `workbench_controller` 仅保留 transport 绑定、协议选择和服务编排。
- UI smoke 测试继续按行为域拆分，单文件上限收紧到 225 行。

## 非目标

- 不新增协议。
- 不改变 UI 交互文案。
- 不改变 uv 启动、测试、打包脚本。
- 不宣称真实硬件验证提升。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 接收 helper 单测、UI smoke 行为域拆分和全量门禁覆盖 |
| 用户 | `U3` | 用户路径不变，连接、发送、接收、导出、回放仍通过既有 UI smoke |
| 设备 | `D2` | 仍为替身和 loopback 证据，真实设备未验证 |

## 验收

- `uv run pytest tests\python\unit\test_controller_receive_state.py -q`
- `uv run pytest tests\python\unit\test_python_governance_docs.py::test_ui_smoke_tests_are_split_by_focused_behavior_domain -q`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug --smoke`
- `cmd /c EmbedDebug.bat --smoke`

## 架构检查

- UI 不直接处理字节流。
- controller 主类不再直接保存测量 ring。
- receive helper 只依赖 controller 值对象、core dispatcher 和测量缓冲工具。
- services、protocols、drivers 依赖方向未改变。
