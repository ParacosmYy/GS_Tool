# PRD-086 - Serial Station Core Dispatcher

## 背景

PRD-085 已经让接收 bytes 经 `SerialStationController` 调用默认协议 `feed()`，形成 RX 日志和计数。但 controller 当前仍直接持有接收协议实例，并负责 reset、feed、事件遍历和半包提示。随着后续协议选择、协议切换、接收 worker 和日志 service 接入，这些职责会继续推高 controller 厚度。

Serial Station 架构文档已明确 `core/SerialDispatcher` 是目标骨架之一。本 PRD 目标是把“原始 bytes 到协议事件”的流式分发职责下沉到 `core/SerialDispatcher`，让 controller 回到 UI 协调和事件转译职责。

## 目标

1. 新增 `src/apps/serial_station/core/SerialDispatcher.h/.cpp`。
2. `SerialDispatcher` 只依赖 `ISerialProtocol` 和 `SerialProtocolEvent`，不依赖 UI、QWidget、具体协议目录或 `SerialStationController`。
3. Dispatcher 支持：
   - 设置当前协议实例。
   - feed 原始 `QByteArray`。
   - reset 当前协议状态。
   - 记录最近一次 feed 的输入字节数和是否形成事件。
4. `SerialStationController` 不再直接持有 `ISerialProtocol` 接收实例，改为持有 `SerialDispatcher`。
5. 保持 PRD-085 的外部行为不变：空 bytes 忽略、半包可见、完整 ASCII 行产生 RX 日志和 RX 计数、connect/disconnect reset。
6. 新增 `tests/serial_station/test_serial_dispatcher.cpp`，覆盖 dispatcher 的协议设置、feed、reset、空 bytes 和半包粘包行为。

## 非目标

1. 不新增 Modbus RTU、Custom MD 或协议选择 UI。
2. 不新增 worker 线程、队列、RingBuffer 或后台调度。
3. 不改变 `SerialManager` 的串口收发接口。
4. 不迁移旧 `src/serial/` 或旧 `src/protocol/`。
5. 不修改 `EmbedDebug.bat`、构建输出目录、Qt 部署路径或启动入口。

## 分层边界

本轮主改动层级为 `core`，附带 controller 接入和测试：

- `core/SerialDispatcher`：只把 bytes 交给当前 `ISerialProtocol`，返回结构化事件和轻量 feed 状态。
- `protocols/`：继续提供 `ISerialProtocol::feed()`，不感知 dispatcher 以外的 UI。
- `SerialStationController`：只负责创建默认协议、交给 dispatcher、把事件转成日志和计数信号。
- UI：不变，不新增 bytes 或协议解析逻辑。

禁止事项：

- Dispatcher include `protocols/ascii_text/` 或任何具体协议目录。
- Dispatcher include UI、QWidget、`SerialStationWindow` 或 `SerialStationController`。
- UI include `core/SerialDispatcher.h`。
- 为 dispatcher 重复实现 RingBuffer、CRC、HexConverter 或日志服务。

## 验收标准

1. `src/apps/serial_station/core/SerialDispatcher.h/.cpp` 存在并加入主 `CMakeLists.txt`。
2. `tests/serial_station/test_serial_dispatcher.cpp` 存在并加入 `tests/CMakeLists.txt`。
3. `SerialStationController` 使用 dispatcher 处理接收 bytes，外部 RX 行为与 PRD-085 保持一致。
4. Dispatcher 单测覆盖：
   - 未设置协议时 feed 返回空事件。
   - 空 bytes 不触发协议 feed。
   - ASCII 完整行产生 frame 事件。
   - 半包后补齐能形成完整事件。
   - reset 后旧半包不会污染新输入。
5. `cmake --build build --target EmbedDebug --parallel 4` 通过。
6. Serial Station 相关 QTest 通过。
7. `tools/doctor.ps1` 通过，允许保留既有 `go.exe` warning。
8. `EmbedDebug.bat` 启动验证通过。

## 失败条件

1. Dispatcher 直接依赖具体协议目录或 UI。
2. Controller 继续直接持有接收协议实例。
3. 新增 `.h/.cpp` 未注册到 CMake。
4. 为 dispatcher 引入重复轮子或第二套协议 registry。
5. 构建失败、测试失败或启动验证失败。
