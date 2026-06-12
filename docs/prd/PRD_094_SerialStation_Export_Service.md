# PRD_094 - Serial Station Export Service

## 1. 背景

Serial Station 已经完成结构化日志服务和 controller 日志接入，但导出功能仍停留在 `SerialLogPanel` 的 UI 占位提示中。按照 Serial Station 架构，文件写入必须收敛到 `services/`，不能散落在 UI 槽函数里。

本阶段新增 `SerialExportService`，承接日志导出文本生成和文件写入错误处理，为后续 UI 导出按钮接入提供稳定服务。

## 2. 目标

1. 在 `src/apps/serial_station/services/` 增加 `SerialExportService.h/.cpp`。
2. 支持从 `SerialLogRecord` 集合导出为 Plain Text、JSON Lines、CSV。
3. 支持写入文件路径并返回结构化结果。
4. 对空路径、空记录、目录不存在、文件无法打开等错误给出明确结果。
5. 增加 `tests/serial_station/test_serial_export_service.cpp`，覆盖格式和错误路径。

## 3. 非目标

- 本阶段不修改 `SerialLogPanel` 或 `SerialStationWindow`。
- 本阶段不打开文件对话框，不做 UI 交互。
- 本阶段不实现回放服务。
- 本阶段不修改串口 core、协议 parser 或 controller。

## 4. 分层边界

本次改动属于 `services` 层。

- `SerialExportService` 可依赖 `SerialLogService` 的数据模型。
- 文件写入只发生在 `services/SerialExportService`。
- 不 include `ui/`、`SerialStationWindow`、`SerialStationController` 或 QWidget。
- 不 include `core/SerialManager`、worker 或具体协议目录。

## 5. 验收标准

- Plain Text 输出复用日志服务稳定格式。
- JSON Lines 每条记录一行，字段和日志服务保持一致。
- CSV 输出包含固定表头：timestamp,direction,source,text,payloadHex。
- CSV 正确转义逗号、引号、换行。
- 文件导出成功时返回成功状态、路径和字节数。
- 空路径、空记录、目录不存在、文件不可写返回失败状态和原因。
- QTest 不依赖真实串口或 UI。
- 新增 `.h/.cpp` 已加入 `CMakeLists.txt`。
- `EmbedDebug.bat` 启动探针仍通过。

## 6. 失败条件

- UI 或 controller 直接写文件。
- service 反向依赖 UI/core/protocols。
- 导出格式靠手写 JSON 拼接导致转义不稳定。
- 写入失败被静默吞掉。
- 新增源码未注册 CMake 或未配套 QTest。
