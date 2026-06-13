# PRD-128 Specs - Serial Station JustFloat 协议基础

## 执行边界

- 主线：PRD 线。
- 主改层：`protocols`。
- 附带触点：`SerialProtocolRegistry`、测试 CMake、主目标 CMake、README、评分追踪。
- 不触碰：`core/SerialManager`、`workers/`、日志导出服务、图表 UI。

## 行为规格

### 协议标识

- `JustFloatProtocol::name()` 返回 `just_float`。
- registry 内置协议列表包含 `just_float`。
- 默认协议仍保持 `ascii_text`。

### 发送构建

- `buildCommand(command, params)` 默认返回 `command.trimmed().toUtf8()`。
- 当 `appendNewline=true` 时追加 `\n`。
- 该发送语义只用于保留串口命令入口，不负责构造二进制 float 帧。

### 接收解析

- 帧尾固定为 `00 00 80 7F`。
- 帧尾之前的 payload 必须非空且长度为 4 的整数倍。
- payload 每 4 字节按小端 IEEE754 单精度浮点解析。
- 每个完整帧返回一个 `measurement` 事件：
  - `event.protocolName = "just_float"`
  - `event.raw = payload + tail`
  - `payload["format"] = "just_float"`
  - `payload["values"] = QVariantList`
  - `payload["channelCount"] = values.size()`
  - `payload["frameIndex"]` 从 1 开始递增
- 半包没有帧尾时缓存，不返回事件。
- 粘包按顺序返回多个事件。
- payload 长度非法时返回 `error` 事件并从帧尾后继续重同步。
- `reset()` 清空缓存并把 frame index 重置为 0。

## 测试清单

- `test_just_float_protocol.cpp`
  - 稳定协议名。
  - 默认命令 trim 后编码。
  - `appendNewline` 可追加换行。
  - 单通道 frame 解析。
  - 多通道 frame 解析。
  - split frame 跨 feed 解析。
  - 单 feed 多 frame 解析。
  - partial frame 未到尾不返回。
  - 非 4 字节 payload 返回 error 并继续解析后续合法帧。
  - `reset()` 清缓存。
  - `reset()` 重置 frame index。
  - 空 bytes 无事件。
  - 负数和小数可解析。
  - raw frame 包含 payload 和 tail。
  - payload 包含 channelCount。
- `test_serial_protocol_registry.cpp`
  - 内置注册包含 `just_float`。
  - 每次 create 返回独立协议实例。
- `test_serial_station_workbench.cpp`
  - 协议选择控件包含 `just_float`。
  - 选择 `just_float` 后 controller 和日志同步。

## 验证命令

```powershell
cmake --build build --target test_just_float_protocol test_serial_protocol_registry test_serial_station_workbench test_startup_options EmbedDebug --parallel 4
.\build\tests\test_just_float_protocol.exe
.\build\tests\test_serial_protocol_registry.exe
.\build\tests\test_serial_station_workbench.exe
.\build\tests\test_startup_options.exe
.\EmbedDebug.bat --station serial
git diff --check
```

## 失败条件

- UI 或 core 直接 include `protocols/just_float/JustFloatProtocol.h`。
- 新协议源码未加入 CMake。
- README 把本轮描述为真实硬件或完整波形能力。
- 默认协议从 `ascii_text` 变成 `just_float`。
