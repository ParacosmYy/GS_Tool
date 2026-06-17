# PRD-135 B3 - Python Protocol Core Specs

> 用途：把 VOFA+ RawData / FireWater / JustFloat 先迁移为 Python 纯协议核心。B3 不接 PyQt UI、不接串口硬件、不做实时绘图。

## 1. 目标

- 新增 Python `ProtocolEvent` 与 `SerialProtocol` 契约。
- 新增 `RawDataProtocol`、`FireWaterProtocol`、`JustFloatProtocol`。
- 新增最小 `SerialProtocolRegistry` 与 `SerialDispatcher`。
- 新增 Python unit/golden tests 覆盖三类协议的流式解析。
- 保持 C++ baseline：`EmbedDebug.bat -> build/EmbedDebug.exe` 不变。
- 本轮三轴状态：工程 `E4自动化测试通过`，用户 `U1可见不可用`，设备 `D1纯单测`。

## 2. 非目标

- 不接 `QSerialPort`。
- 不新增 PyQt UI。
- 不新增 pyqtgraph、NumPy ring buffer 或实时曲线。
- 不新增 PyInstaller spec。
- 不修改 C++ 源码、CMake、README 或 `EmbedDebug.bat`。
- 不声称 Python 串口工作流用户可用。

## 3. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| 主要文件 | `python/embeddebug/serial_station/protocols/` | 新增/更新 |
| 主要文件 | `python/embeddebug/serial_station/core/` | 新增/更新 |
| 主要文件 | `tests/python/unit/` | 新增 |
| 主要文件 | `docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B3_Protocol_Core_Specs.md` | 新增 |
| 禁止修改 | `src/**` | 禁止 |
| 禁止修改 | `cmake/**` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 4. Protocol Semantics

- `raw_data`: emits one `frame` event per non-empty feed, preserving exact bytes and best-effort UTF-8 text.
- `fire_water`: parses newline-delimited numeric frames, supports CRLF/LF/CR, optional prefix before `:`, optional header names, invalid numeric fields as `nan`, and exact raw line preservation.
- `just_float`: parses little-endian float32 frames terminated by `00 00 80 7F`, supports split frames, multiple frames, invalid payload-length errors, and `reset()`.

## 5. 验收标准

- [ ] `uv run test-embeddebug-py` 通过。
- [ ] `uv run test-embeddebug-tools` 通过。
- [ ] `uv run start-embeddebug-py --smoke` 通过。
- [ ] 协议模块不 import `PyQt6`。
- [ ] Python 源码只在 `python/embeddebug/` 下。
- [ ] Python 测试只在 `tests/python/` 下。
- [ ] C++ 源码、CMake、`EmbedDebug.bat` 未修改。

## 6. GO 配置

```powershell
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug-py --smoke
rg "PyQt6" python/embeddebug/serial_station/protocols python/embeddebug/serial_station/core
git diff --check -- python tests/python docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B3_Protocol_Core_Specs.md
```

`rg "PyQt6" ...` 预期无匹配。

## 7. 收口状态

| 轴 | 状态 | 说明 |
|----|------|------|
| Engineering | E4 | pure Python protocol tests pass |
| User | U1 | Python app shell exists, no serial workflow |
| Device | D1 | pure unit/golden tests only |
