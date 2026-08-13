# ADR 0011：历史 RX 回放复用现有派生链

状态：已接受（M5c，2026-08-09）

## 背景

SerialForge 已经有 recorder JSONL、framing/checksum、schema v2 component codec 和 bounded
Dataset worker。下一步需要在没有设备和 J-Link 驱动时复现问题、检查时间序列和验证配置，
但不能因此复制一套“回放专用解析器”，也不能把历史数据伪装成实时设备。

## 决策

新增独立的 `ReplayPipelinePort` 和 `ReplayPipelineWorker`：

- `domain/replay.py` 负责严格 recorder JSONL 记录契约、`ReplayOptions`、生命周期和预算快照；
- `application/replay.py` 只做有界文件读取、捕获时间轴调度、暂停/恢复/停止和 protocol ingress；
- 只接受 RX 的 UART/TCP Client stream；TX、UDP、BLE、RTT 和无效记录跳过或记录为可见计数；
- 通过 `ProtocolSource.origin=DataOrigin.HISTORICAL` 和独立 replay UUID 隔离实时 session；
- 回放 raw payload 经过原有 `ProtocolPipelineWorker → ComponentPipelineWorker → DatasetPipelineWorker`，
  不在 UI 解析 JSONL，不执行用户代码，不重新写入 recorder；
- 行读取、单行大小、文件字节、记录数量、protocol queue 和下游 retained window 均保持有界；
- 调度以原始 `occurred_at` 差值表达历史时间，以 `time.monotonic()` 实际等待；暂停冻结等待，
  protocol 背压重试或在 stop 时退出。

## UI 边界

M5c 只提供速度、选择并回放、暂停、停止和状态/计数预览。历史数据会在 source label 中标记，
实时连接和发送控件在回放期间禁用。曲线绘制、时间轴缩放、批量回放筛选和回放导出另行设计，
避免现在引入图表运行时或把 `MainWindow` 变成数据处理中心。

## 六角色只读复核

| 角色 | 子代理 run | 结论/证据 |
|---|---|---|
| 产品 | `019fe45b-0e50-7c73-90e4-a8c9b221c576` | revise；限定 M5c 为严格 JSONL RX 回放、暂停/停止/EOF，不做 TX、重录、数据库和图表依赖 |
| 架构 | `019fe45b-0e88-7051-a964-db77244f6ed9` | revise；要求 replay port 与 transport 分离、historical source/generation 隔离、保留 bounded snapshot |
| UI 设计 | `019fe45b-0ec3-7f11-9f48-13ca0f76ce87` | revise；建议 Monitor/Config/Replay 的后续布局，M5c 使用内置 bounded preview，不新增 chart 依赖 |
| 开发 | `019fe45b-0f00-7bd3-990a-02c1402060c6` | revise；确认新增 domain/application replay 边界并复用 protocol/component/dataset chain |
| 验证 | `019fe45b-0f41-70b0-95d4-1effa36d2565` | revise；提出时间戳、base64、暂停时钟、取消、队列 cap、worker leak 和 offscreen/package 验证矩阵 |
| 打包/流程 | `019fe45b-0f7a-7fb2-8ebd-b2ddb00c22d2` | revise；确认无新增运行时依赖，提醒默认 BLE-free 包扫描与许可证/CI 门 |

六个子代理均为当前 checkout 的只读复核，父代理负责唯一源码写入、整合和最终验证；已关闭。

## 验证证据

- `uv lock --check`、Ruff format/check、`compileall` 和 `scripts/check.ps1`：通过；
- 临时内存/JSONL 向量：严格字段解码、RX-only、TX/UDP 跳过、坏 base64、非单调时间、历史 source、
  protocol/component/dataset 端到端派生：通过；
- 临时回放控制向量：速度调度、暂停冻结、恢复、停止、协议背压重试：通过；
- 超长 JSONL 行向量：读取限制在 `MAX_REPLAY_LINE_BYTES + 1`，不把整行载入内存：通过；
- Qt `offscreen`：创建主窗口、触发历史回放、窗口关闭及 replay/protocol/component/dataset/recorder
  worker shutdown：通过；已知 Qt 缺少字体目录提示不影响退出；
- 未创建或修改单元测试、mock、fixture、测试 harness 或其他 test-only asset。

## 后续风险

- 尚未在真实 UART/TCP 长时间吞吐或损坏/超大记录压力下验收；
- 现有 recorder JSONL 是固定 schema，未来 schema 变化需要显式版本/迁移策略；
- DatasetBatchEvent 仍是 bounded preview，曲线视图需要独立的批量/采样策略；
- J-Link RTT 仍保持最后能力，驱动下载完成后另行做 attach-only 硬件验证，不由 M5c 代替。

## 简化评估

回放只新增一个 domain contract 和一个 application worker，复用现有 source-aware protocol、
component、Dataset 和事件总线；没有引入数据库、图表库、插件运行时或第二套 codec。历史来源
通过类型化 DTO 和 source gate 显式表达，避免在每个下游组件中增加“是否回放”的隐式布尔分支。
