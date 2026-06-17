# PRD-192 Serial Station 连接工具栏拆分

## 目标

把 `sections.py` 中连接、协议、串口参数、TCP/UDP 入口的工具栏装配拆到独立 UI helper，降低主布局文件行数压力，继续保持 PyQt 上位机 UI 层可维护、可迭代。

## 范围

- 新增 `connection_toolbar` UI helper。
- `sections.build_main_layout` 只负责主布局编排，不直接堆叠连接工具栏控件细节。
- 连接、协议选择、串口参数、TCP/UDP、断开按钮的 objectName、tooltip、信号连接和默认状态保持不变。
- 不改变 controller、transport、profile、日志或协议行为。

## 非目标

- 不新增连接类型。
- 不改变 UI 文案。
- 不改变快捷键。
- 不新增脚本、打包链路或 native 入口。

## 架构边界

- helper 位于 `python/embeddebug/serial_station/ui/`，只做 UI 控件装配。
- helper 不直接调用 controller 连接结果方法，只连接 owner 暴露的 UI 意图方法。
- `sections.py` 保留根页面、日志、波形、profile、footer 的整体编排职责。

## 验收标准

- 架构测试证明 `sections.build_main_layout` 通过 `build_connection_toolbar` 委托连接工具栏。
- `sections.py` 低于 260 行。
- 现有 UI smoke 行为保持通过。
- 完整门禁通过：`uv run test-embeddebug-py`、`uv run test-embeddebug-tools`、`uv run start-embeddebug --smoke`、`cmd /c EmbedDebug.bat --smoke`。

## 三轴状态

| 维度 | 目标 |
|---|---|
| 工程状态 | `E4`，架构测试与 UI smoke 覆盖 |
| 用户状态 | `U3`，用户连接入口行为不变 |
| 设备状态 | `D2`，替身链路保持 |
