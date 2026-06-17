# PRD-175 Serial Station Profile Combo Options Helper

## 1. 目标

将 Profile 恢复时下拉框补项与选中规则从 `session_actions.py` 中抽离到独立 UI helper，避免 session action 维护 Combo 细节。

## 2. 范围

- 新增 `ui/profile_combo_options.py`。
- 提供 `select_profile_combo_value()`。
- `session_actions.apply_profile_controls()` 复用 helper。
- 补充纯单测覆盖已有选项选中与缺失选项补项两条路径。

## 3. 非目标

- 不改变 Profile 文件格式、保存策略、加载流程或连接状态同步。
- 不改变 TCP/UDP endpoint 写入、controller、transport、protocol 或 service 行为。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/profile_combo_options.py` | 统一 Profile 下拉框补项与选中规则 |
| `ui/session_actions.py` | 响应 Profile 加载并委托下拉控件 helper |
| `controllers/` / `services/` | 提供 Profile 数据，不直接操作 QWidget |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | Profile 恢复后的下拉选项显示行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_profile_combo_options.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
