# PRD_097 - Serial Station UI Replay Flow

## 1. 背景

Serial Station 已具备结构化日志、日志导出和服务层回放计划生成能力，但用户还不能从工作台直接触发回放预览。`SerialReplayService` 当前停留在 services 层，尚未通过 controller 暴露给 UI。

本阶段把回放服务接入到工作台日志面板，形成“用户点击回放 -> Controller 生成回放计划 -> UI 日志展示计划摘要”的可见闭环。该闭环只做回放预览，不实际向串口写入数据。

## 2. 目标

1. 在日志面板增加“回放”用户入口。
2. 日志面板只发出 replay 意图，不直接调用 service/core/protocol。
3. `SerialStationController` 调用 `SerialReplayService` 基于当前结构化日志生成回放计划。
4. Controller 将计划摘要和前几条事件摘要写回系统日志。
5. 空日志、过滤后无事件等失败路径给出明确系统日志。
6. 增加 controller/UI 层 QTest，覆盖按钮存在、信号闭环、成功和失败路径。
7. 检查 README 是否需要同步宣传入口；如本阶段新增用户可见能力，应补充 Serial Station 能力描述。

## 3. 非目标

- 不真实发送回放帧到串口。
- 不新增后台 worker 或定时播放队列。
- 不读取外部日志文件。
- 不新增协议解析逻辑。
- 不修改 `core/SerialManager` 的发送/接收循环。
- 不引入 native 弹窗确认；反馈只通过工作台日志和状态文本完成。

## 4. 分层边界

本次改动跨 `ui`、`controller`、`services`，调用链必须固定为：

```text
SerialLogPanel::replayRequested()
  -> SerialStationWindow
  -> SerialStationController::previewReplayPlan()
  -> SerialReplayService::buildPlan()
  -> SerialStationController emits serialSystemLogged()
  -> SerialLogPanel::appendSystem()
```

边界要求：

- UI 不 include `SerialReplayService`，不读取结构化日志仓库。
- Window 只做 signal/slot 装配和用户意图转发。
- Controller 只做协调、错误汇总和系统日志输出。
- Service 继续只做计划生成，不接触 QWidget、SerialManager 或 worker。
- 协议层和 core 层不参与本阶段。

## 5. 验收标准

- 日志面板存在 objectName 稳定的回放按钮。
- 点击回放按钮会通过 signal 进入 Controller。
- 当前日志为空时，日志区展示“回放预览失败”类系统反馈。
- 已有 TX/RX 结构化日志时，回放预览生成 ok plan。
- 预览日志包含事件数量、总耗时、跳过数量和至少一条事件摘要。
- 清空日志后再回放不会复用旧计划。
- 所有用户可见文字使用 `tr()`。
- 新增 `.h/.cpp` 如果存在，必须注册 CMake；本阶段优先复用现有文件，避免不必要文件扩张。
- README 对 Serial Station 能力描述包含日志导出/回放预览，作为企业级宣传入口保持同步。

## 6. 失败条件

- UI 直接创建或调用 `SerialReplayService`。
- 回放按钮直接调用 `SerialManager::send()`。
- Controller 将回放实现写成真实串口播放。
- 失败路径静默无反馈。
- 新增测试依赖真实 COM 口、人工操作或 native 文件对话框。
- README 在新增用户可见能力后仍未检查或未说明原因。
