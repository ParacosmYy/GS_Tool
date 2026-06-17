# PRD-135 B5 - Python Transport Core Specs

> 用途：建立 Python/PyQt lane 的传输抽象、fake transport 和 QSerialPort thin adapter。B5 不接 UI、不要求真实 COM 口。

## 1. 目标

- 新增 Python `SerialTransport` 契约和 `SerialPortConfig`。
- 新增 `FakeSerialTransport`，用于替身收发、错误路径和 D2 起点验证。
- 新增 `QtSerialPortTransport` thin adapter，封装 PyQt6 `QSerialPort` 基础配置、打开、关闭、写入和端口枚举。
- 新增 transport unit tests。
- 保持 C++ baseline：`EmbedDebug.bat -> build/EmbedDebug.exe` 不变。
- 本轮三轴状态：工程 `E4自动化测试通过`，用户 `U1可见不可用`，设备 `D2替身验证`。

## 2. 非目标

- 不打开真实 COM 口。
- 不做 Windows 虚拟串口对测试。
- 不接 PyQt UI 控件。
- 不实现自动重连、DTR/RTS 或高级串口配置。
- 不新增 PyInstaller spec。
- 不修改 C++ 源码、CMake、README 或 `EmbedDebug.bat`。

## 3. 改动范围

| 范围 | 路径 | 动作 |
|------|------|------|
| 主要文件 | `python/embeddebug/serial_station/drivers/` | 新增/更新 |
| 主要文件 | `tests/python/unit/` | 新增 |
| 主要文件 | `docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B5_Transport_Core_Specs.md` | 新增 |
| 禁止修改 | `src/**` | 禁止 |
| 禁止修改 | `cmake/**` | 禁止 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |

## 4. 验收标准

- [ ] `uv run test-embeddebug-py` 通过。
- [ ] `uv run test-embeddebug-tools` 通过。
- [ ] `uv run start-embeddebug-py --smoke` 通过。
- [ ] fake transport 测试覆盖 open/write/inject/read/close/error。
- [ ] QSerialPort adapter 不要求真实串口即可实例化和枚举端口。
- [ ] Python 源码只在 `python/embeddebug/` 下。
- [ ] Python 测试只在 `tests/python/` 下。
- [ ] C++ 源码、CMake、`EmbedDebug.bat` 未修改。

## 5. GO 配置

```powershell
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug-py --smoke
git diff --check -- python tests/python docs/superpowers/specs/PRD_135_Python_PyQt_Migration_B5_Transport_Core_Specs.md
```

本轮不执行真实串口或虚拟 COM 验证。

## 6. 收口状态

| 轴 | 状态 | 说明 |
|----|------|------|
| Engineering | E4 | transport unit tests pass |
| User | U1 | Python app shell exists, no UI workflow |
| Device | D2 | fake transport validates substitute I/O |
