# PRD-173 Serial Station Profile Name Text Helper

## 1. 目标

将 Profile 加载后写入名称输入框的规则从 `session_actions.py` 中抽离到独立 UI helper，避免 session action 直接维护输入控件写入细节。

## 2. 范围

- 新增 `ui/profile_name_text.py`。
- 提供 `apply_profile_name_text()`。
- `session_actions.load_profile()` 复用 helper。
- 补充纯单测覆盖名称写入与空名称清理两条路径。

## 3. 非目标

- 不改变 Profile 文件格式、保存策略或加载流程。
- 不改变 controller、transport、protocol 或 service 行为。
- 不调整 `EmbedDebug.bat`、uv scripts、PyInstaller 打包链路。

## 4. 架构边界

| 层 | 责任 |
|---|---|
| `ui/profile_name_text.py` | 统一 Profile 名称输入框文本写入规则 |
| `ui/session_actions.py` | 响应 Profile 加载并委托 helper |
| `controllers/` | 提供 Profile 数据，不直接操作输入控件 |

## 5. 三轴状态

| 维度 | 本批目标 | 证据 |
|---|---|---|
| 工程状态 | `E4` | 单测、UI 架构测试、UI smoke 与完整 Python 门禁通过 |
| 用户状态 | `U3` | Profile 加载后的名称输入显示行为保持一致 |
| 设备状态 | `D2` | 维持已有替身/loopback 证据 |

## 6. 验收

```powershell
uv run pytest tests\python\unit\test_profile_name_text.py tests\python\unit\test_serial_station_ui_architecture.py tests\python\ui_smoke\test_serial_station_mvp.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
