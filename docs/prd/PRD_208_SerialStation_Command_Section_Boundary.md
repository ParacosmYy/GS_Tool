# PRD-208 Serial Station Command Section Boundary

## 目标

将 Serial Station UI 的命令发送行从主 layout builder 中拆出，避免 `sections.py` 同时承担窗口骨架和具体命令控件装配。

## 范围

- 新增 `command_section`，承接命令输入框、命令历史下拉和发送按钮构建。
- `sections.build_main_layout` 只调用 command section builder，不直接持有发送控件细节。
- 更新 UI 架构测试，防止命令发送行构建职责回流到 `sections.py`。

## 非目标

- 不改变命令发送、命令历史或快捷键行为。
- 不改变 `command_actions` 的发送逻辑。
- 不新增协议、transport 或真实设备能力。
- 不提升真实设备验证口径。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | UI 架构测试锁定 command section 边界，主 sections 文件降到 152 行 |
| 用户 | `U3` | 命令输入、历史选择和发送按钮入口不变 |
| 设备 | `D2` | 维持替身/loopback 验证口径，真实设备未验证 |

## 验收

- `uv run pytest tests\python\unit\test_serial_station_ui_architecture.py -q`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug --smoke`
- `cmd /c EmbedDebug.bat --smoke`

## 架构检查

- `command_section` 只构建 UI 控件，不访问 controller、transport、protocol 或 service。
- `sections.py` 保持主 layout 编排职责，不承接命令控件细节。
- `main_window` 仍只做 action 委托，不增加业务逻辑。
- UI、controller、drivers、core、protocols、services 依赖方向未改变。
