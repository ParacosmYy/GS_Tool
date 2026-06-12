# PRD_095 - Serial Station UI Export Flow

## 1. 背景

Serial Station 已经具备结构化日志服务和 `SerialExportService` 文件导出能力，但 `SerialLogPanel` 的“导出”按钮仍只发出占位提示，用户无法从工作台完成真实日志导出。

本阶段把导出服务接入 `SerialStationController` 和 `SerialStationWindow`，形成 UI 导出闭环：用户点击导出、选择文件路径、controller 调用 service 写入文件、结果回写系统日志。

## 2. 目标

1. `SerialStationController` 暴露日志导出方法，内部调用 `SerialExportService`。
2. `SerialStationWindow` 响应 `SerialLogPanel::exportRequested`，打开保存文件对话框并构造导出请求。
3. UI 默认导出 JSON Lines，支持根据文件后缀选择 Plain Text、JSON Lines、CSV。
4. 导出成功或失败都通过 controller 结构化日志回写，不使用占位文案。
5. 增加 QTest 覆盖 controller 导出成功、失败、格式选择和结果日志。

## 3. 非目标

- 本阶段不新增导出格式。
- 本阶段不实现日志回放。
- 本阶段不修改串口 `core/` 收发逻辑。
- 本阶段不把文件写入逻辑放到 UI。
- 本阶段不重做 `SerialLogPanel` 视觉样式。

## 4. 分层边界

本次改动涉及 `ui -> controller -> services` 三层，跨层原因是已有 service 需要被用户入口调用。

- `SerialLogPanel` 只发出 `exportRequested` 信号。
- `SerialStationWindow` 只负责 UI 层路径选择、取消处理和请求参数组装。
- `SerialStationController` 只负责读取日志快照、调用 `SerialExportService`、记录成功/失败系统日志。
- `SerialExportService` 继续作为唯一文件写入点。
- `ui/` 不 include `SerialExportService`，不直接写文件。
- `services/` 不 include QWidget 或 UI 头文件。

## 5. 验收标准

- 点击导出后不再出现“后续阶段接入”的占位系统日志。
- 用户取消文件对话框时，工作台记录“日志导出已取消”类系统日志。
- 无日志记录时导出失败，错误原因可见。
- 成功导出后 controller 记录成功日志，包含路径、格式和字节数。
- `.jsonl`、`.txt`、`.csv` 后缀能映射到对应格式；无后缀时补 `.jsonl`。
- 文件写入仍只发生在 `SerialExportService`。
- 新增测试不依赖真实串口或人工文件对话框。
- Serial Station 分层检查通过。
- README 已检查：README 当前已经覆盖 Serial Station 日志导出与工程质量说明，本阶段不改变对外构建/启动命令，暂不需要单独更新。

## 6. 失败条件

- UI 直接调用 `SerialExportService` 或直接写文件。
- `SerialLogPanel` 持有 controller/service 指针。
- controller 塞入格式化细节或重复实现 CSV/JSON。
- 导出错误被静默吞掉。
- 新增测试依赖真实 COM 口或需要人工操作文件对话框。
- 修改导致 `SerialStationController.cpp` 超过 300 行且没有拆分说明。
