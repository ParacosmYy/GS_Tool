# PRD_093 - Serial Station Controller Log Service Integration

## 1. 背景

PRD_092 已经落地 `services/SerialLogService`，但当前 `SerialStationController` 仍只向 UI 发出字符串日志信号，日志事实没有进入结构化服务仓库。

这会导致：

- 后续导出、回放、审计仍缺少统一日志来源。
- UI 可以看到日志，但 controller 侧无法查询本次会话的 TX/RX/System/Error 历史。
- `SerialLogService` 处于孤立状态，未形成 Serial Station 本体闭环。

本阶段让 `SerialStationController` 持有并写入 `SerialLogService`，同时保留现有 UI signal 兼容行为。

## 2. 目标

1. `SerialStationController` 持有 `SerialLogService`。
2. Controller 在连接、发送失败、发送成功、接收帧、接收缓存、未知事件等路径写入结构化日志。
3. 对外提供只读日志查询、纯文本和 JSON Lines 导出预览接口。
4. 提供 controller 级清空日志入口，供后续 UI 清空动作接入。
5. 增加独立 QTest，验证 controller 与 log service 的接入行为。

## 3. 非目标

- 本阶段不改 `SerialLogPanel` 的内部存储。
- 本阶段不修改 UI 布局、QSS、按钮或导出文件对话框。
- 本阶段不写真实文件，不实现 `SerialExportService`。
- 本阶段不修改串口 core、协议 parser 或 worker。

## 4. 分层与调用链

本次改动跨 `controller + services`，调用链如下：

```text
UI signal / SerialManager bytes
  -> SerialStationController
  -> SerialLogService append/query
  -> SerialStationController 原有 UI signal
  -> SerialLogPanel / SerialStatusBar
```

边界要求：

- UI 不直接持有 `SerialLogService`。
- `SerialLogService` 不 include UI、core、protocols 具体目录或 QWidget。
- Controller 只做协调与日志事实归档，不把导出文件 IO 写进 controller。
- 原有 UI 日志信号保持兼容，已有工作台测试不应回归。

## 5. 验收标准

- 发送失败会写入 `Error` 方向日志，且仍发出 `serialSystemLogged` 与 `serialCommandFailed`。
- 接收完整帧会写入 `Rx` 方向日志，payload 保留原始 bytes。
- 接收半包缓存提示会写入 `System` 方向日志。
- 连接配置日志会写入 `System` 方向日志。
- controller 暴露的 plain text / JSON Lines 能反映当前结构化日志。
- 清空 controller 日志后查询为空，不影响后续继续追加。
- 新增测试不依赖真实 COM 口。
- 既有 Serial Station 回归测试继续通过。
- `EmbedDebug.bat` 启动探针继续通过。

## 6. 失败条件

- UI 直接 include 或直接调用 `SerialLogService`。
- `SerialLogService` 反向依赖 controller、core、protocols 或 QWidget。
- 为了日志接入修改协议解析逻辑。
- 破坏已有 `serialTxLogged`、`serialRxLogged`、`serialSystemLogged` 信号语义。
- 新增代码提交未达到约束规定的代码增量。
