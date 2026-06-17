# PRD-166 Serial Station Profile Label Helper

## 1. 目标

将 Profile 保存与加载路径中的 Profile 标签展示收敛到 `ui/status_messages.py`，减少 `session_actions.py` 中直接格式化控件文本的重复逻辑。

## 2. 范围

- 新增 `set_profile_label()` 公共 UI helper。
- `session_actions.py` 的保存 Profile 与加载 Profile 成功路径复用公共 helper。
- 补充纯单测覆盖翻译与 Profile 名称格式化。
- 保持 Profile 保存、加载、控件回填、命令历史刷新和状态栏反馈行为不变。

## 3. 非目标

- 不改变 Profile 文件格式、持久化 service 或 controller 行为。
- 不新增连接类型、协议能力或真实硬件验证。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/status_messages.py` | 统一普通状态、结果状态和 Profile 标签展示 |
| `ui/session_actions.py` | 编排保存/加载动作并调用公共展示 helper |
| `controllers/` / `services/` | 保持业务结果与持久化职责，不写 UI 文案 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | Profile 保存/加载可见反馈保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_status_messages.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_workflow.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
