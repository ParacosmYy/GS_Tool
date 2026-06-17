# PRD-135 B4 - Python Service Core Specs

> 用途：把 Serial Station 的日志、导出、回放、Profile 先迁移为 Python 纯服务层。B4 不接 UI、不接串口硬件、不做 PyInstaller。

## 1. 目标

- 新增 Python `SerialLogService`，把 `ProtocolEvent` 追加为 JSON Lines。
- 新增 Python `SerialReplayService`，从 JSON Lines 恢复事件。
- 新增 Python `SerialMeasurementExportService`，把 measurement events 导出 CSV。
- 新增 Python `SerialProfileService`，保存/加载 profile JSON。
- 新增 Python service unit tests。
- 保持 C++ baseline：`EmbedDebug.bat -> build/EmbedDebug.exe` 不变。
- 本轮三轴状态：工程 `E4自动化测试通过`，用户 `U1可见不可用`，设备 `D1纯单测`。

## 2. 非目标

- 不接 `QSerialPort`。
- 不接 PyQt UI。
- 不实现文件选择对话框或用户交互。
- 不新增 PyInstaller spec。
- 不修改 C++ 源码、CMake、README 或 `EmbedDebug.bat`。

## 3. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| 主要文件 | `python/embeddebug/serial_station/services/` | 新增/更新 |
| 主要文件 | `tests/python/unit/` | 新增 |
| 主要文件 | `docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B4_Service_Core_Specs.md` | 新增 |
| 禁止修改 | `src/**` | 禁止 |
| 禁止修改 | `cmake/**` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 4. 验收标准

- [ ] `uv run test-embeddebug-py` 通过。
- [ ] `uv run test-embeddebug-tools` 通过。
- [ ] `uv run start-embeddebug-py --smoke` 通过。
- [ ] 服务模块不 import `PyQt6`。
- [ ] Python 源码只在 `python/embeddebug/` 下。
- [ ] Python 测试只在 `tests/python/` 下。
- [ ] C++ 源码、CMake、`EmbedDebug.bat` 未修改。

## 5. GO 配置

```powershell
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug-py --smoke
rg "PyQt6" python/embeddebug/serial_station/services
git diff --check -- python tests/python docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B4_Service_Core_Specs.md
```

`rg "PyQt6" ...` 预期无匹配。

## 6. 收口状态

| 轴 | 状态 | 说明 |
|----|------|------|
| Engineering | E4 | service unit tests pass |
| User | U1 | Python app shell exists, no service UI |
| Device | D1 | pure unit tests only |
