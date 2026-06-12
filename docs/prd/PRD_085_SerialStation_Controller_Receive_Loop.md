# PRD-085 - Serial Station Controller Receive Loop

## 背景

PRD-084 已经把命令发送从 UI 本地预览推进到 `SerialStationController -> SerialManager -> protocols` 的第一版发送闭环。但接收方向仍停留在 `SerialManager::bytesReceived` 的底层信号，没有经过 controller 和协议层统一解析，也没有回写到工作台日志和 RX 计数。

对于串口上位机来说，只能发不能看见协议化接收结果，仍不是完整调试闭环。本 PRD 目标是补齐第一版接收闭环：串口 bytes 进入 controller，controller 使用默认协议做流式 feed，把协议事件转换成 UI 可见日志和状态计数。

## 目标

1. `SerialStationController` 订阅 `SerialManager::bytesReceived`。
2. Controller 持有或创建当前接收协议实例，调用 `ISerialProtocol::feed(bytes)` 完成流式解析。
3. ASCII 文本协议的完整行事件要转换为 RX 日志。
4. 收到原始 bytes 但暂未形成完整事件时，要有可观察的原始接收日志或调试日志，避免数据静默消失。
5. 解析成功时增加 RX 计数；解析异常或事件类型异常时增加错误计数并发系统日志。
6. `SerialStationWindow` 只连接 controller 信号到 `SerialLogPanel` 和 `SerialStatusBar`，不解析 bytes。
7. 新增或更新 QTest 覆盖 controller 接收路径和 workbench UI 回写。

## 非目标

1. 不新增 Modbus RTU、Custom MD 或二进制协议。
2. 不新增 worker 线程、接收队列、DMA/RingBuffer 或后台 dispatcher。
3. 不做日志文件持久化、导出、回放。
4. 不迁移旧 `src/serial/` 或旧 `src/protocol/`。
5. 不修改 `EmbedDebug.bat`、构建输出目录、Qt 部署路径或启动入口。

## 分层边界

本轮主改动层级为 `controller`，附带 `ui` 装配和测试：

- Core：继续只发出 `QByteArray`，不理解协议事件。
- Protocols：继续只返回 `SerialProtocolEvent`，不写 UI 文案。
- Controller：把 bytes 交给协议 feed，并把事件转换为日志/计数信号。
- UI：只展示 controller 回传结果，不 include core/protocol 头文件。

禁止事项：

- UI include `core/SerialManager.h`、`ISerialProtocol.h` 或具体协议头。
- Core include 具体协议目录。
- Protocols include UI、`SerialStationWindow` 或 `SerialStationController`。
- Controller 为本轮接收闭环新增第二套协议注册表或重复解析器。

## 验收标准

1. `SerialManager::bytesReceived` 连接到 `SerialStationController` 的接收处理入口。
2. 完整 ASCII 行输入如 `OK\n` 能产生 RX 日志并增加 RX 计数。
3. 半包输入如 `O` 不丢失，后续补齐 `K\n` 后能形成完整 RX 日志。
4. 空 bytes 不产生日志和计数。
5. Workbench 测试能验证 controller 接收信号回写日志区和状态栏。
6. `cmake --build build --target EmbedDebug --parallel 4` 通过。
7. Serial Station 相关 QTest 通过。
8. `tools/doctor.ps1` 通过，允许保留既有 `go.exe` warning。
9. `EmbedDebug.bat` 启动验证通过。

## 失败条件

1. UI 直接解析 bytes 或调用协议对象。
2. Core 直接依赖 `ascii_text/` 具体协议目录。
3. 为接收闭环重复实现 RingBuffer、CRC、HexConverter 或日志服务。
4. 新增 `.h/.cpp` 未注册到 CMake。
5. 构建失败、测试失败或启动验证失败。
