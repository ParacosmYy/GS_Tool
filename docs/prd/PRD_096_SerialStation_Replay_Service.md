# PRD_096 - Serial Station Replay Service

## 1. 背景

Serial Station 已具备结构化日志记录、文件导出和 UI 导出闭环，但 `services/` 中仍缺少回放服务。当前日志只能被查看或导出，无法转换成后续“重放发送/接收过程”的稳定数据计划。

本阶段先在服务层落地 `SerialReplayService`，把 `SerialLogRecord` 或 JSON Lines 文本转换为可回放事件序列。服务只负责解析、过滤、时间间隔计算和播放计划生成，不触碰 UI 和串口发送。

## 2. 目标

1. 在 `src/apps/serial_station/services/` 新增 `SerialReplayService.h/.cpp`。
2. 支持从 `QVector<SerialLogRecord>` 构建回放计划。
3. 支持从日志 JSON Lines 文本解析回放计划。
4. 支持按方向过滤，默认只回放 TX/RX，可显式包含 System/Error。
5. 支持基于记录 timestamp 计算相邻事件延迟，并提供速度倍率和最大延迟钳制。
6. 支持将回放事件格式化为摘要文本，便于后续 UI 或 worker 接入。
7. 增加 `tests/serial_station/test_serial_replay_service.cpp` 覆盖正常路径和错误路径。

## 3. 非目标

- 本阶段不修改 UI。
- 本阶段不修改 `SerialStationController`。
- 本阶段不实际写串口、不调用 `SerialManager::send()`。
- 本阶段不新增后台 worker。
- 本阶段不读取文件路径，只接收内存中的记录或 JSON Lines 文本。

## 4. 分层边界

本次改动只属于 `services` 层。

- `SerialReplayService` 可依赖 `SerialLogService` 的结构化日志模型。
- 服务层只处理数据转换，不操作 QWidget。
- 服务层不 include `core/SerialManager.h`、worker 或具体协议目录。
- 解析 JSON Lines 时只还原日志事实，不做协议语义解析。
- 后续 UI/controller 接入必须另开 PRD。

## 5. 验收标准

- 空记录返回失败结果和明确错误原因。
- 默认只生成 TX/RX 方向事件。
- 可通过选项包含 System/Error 事件。
- 事件 payload、方向、source、text、timestamp 保持与日志记录一致。
- 延迟按相邻事件 timestamp 差值计算，首条延迟为 0。
- 速度倍率能缩放延迟，非法倍率按 1.0 处理。
- 最大延迟能钳制过长间隔。
- JSON Lines 解析能读取 `SerialLogService::toJsonLines()` 输出。
- 单行 JSON 错误、缺少 direction/text 的输入返回失败。
- QTest 不依赖真实串口、UI 或文件系统。
- 新增 `.h/.cpp` 已加入主 `CMakeLists.txt` 和测试目标。

## 6. 失败条件

- service 直接发送串口或持有 `SerialManager`。
- service include UI、controller、core 或具体协议目录。
- JSON 解析错误被静默跳过。
- 时间间隔出现负数或整数溢出。
- 新增源码未注册 CMake。
- 测试需要真实 COM 口或人工操作。
