# M5c 六角色复核记录：历史 RX 回放

日期：2026-08-09  
范围：严格 recorder JSONL replay、历史来源隔离、protocol/component/Dataset 复用、暂停/停止、
有界读取、UI 状态和 PyInstaller 交付。  
源码写入者：父代理；六角色子代理均为只读复核，无子代理直接修改当前 checkout。

## 六角色记录

| 角色 | 子代理 run | 结论 | 关键建议 |
|---|---|---|---|
| 产品 | `019fe45b-0e50-7c73-90e4-a8c9b221c576` | revise → 已整合 | M5c 只做严格 JSONL RX 回放、暂停/停止/EOF；不做 TX、重录、数据库、图表依赖 |
| 架构 | `019fe45b-0e88-7051-a964-db77244f6ed9` | revise → 已整合 | replay port 与 transport 分离；historical origin、generation/队列和 bounded snapshot 必须明确 |
| UI 设计 | `019fe45b-0ec3-7f11-9f48-13ca0f76ce87` | revise → 已整合 | 后续可拆 Monitor/Config/Replay；当前使用内置速度/暂停/停止/计数预览，不引入 chart 依赖 |
| 开发 | `019fe45b-0f00-7bd3-990a-02c1402060c6` | revise → 已整合 | `domain/replay.py` + `application/replay.py`，复用 protocol/component/dataset chain |
| 验证 | `019fe45b-0f41-70b0-95d4-1effa36d2565` | revise → 已整合 | 覆盖时间戳、base64、RX-only、暂停时钟、取消、背压、队列 cap、worker leak、offscreen/package |
| 打包/流程 | `019fe45b-0f7a-7fb2-8ebd-b2ddb00c22d2` | revise → 已整合 | 无新增运行时依赖；默认包保持 BLE-free；许可证/CI/onefile 作为交付门单独记录 |

六个子代理已关闭；父代理负责整合、最终源码检查和验证。

## 变更摘要

- `DataOrigin.LIVE/HISTORICAL` 进入 `ProtocolSource`，历史 source display 明确标记；
- `ProtocolIngressUnit.occurred_at` 保留回放捕获时间，protocol frame event 复用该时间；
- 新增 `ReplayOptions`、`ReplaySnapshot`、`ReplayRecordCodec` 和 `ReplayPipelinePort`；
- `ReplayPipelineWorker` 读取固定 recorder JSONL，拒绝重复 key/非有限数字/无时区/坏 UUID/坏 base64/超限 payload，
  只投递 UART/TCP Client RX；TX、UDP、BLE、RTT 和无效行可见跳过/错误计数；
- replay 通过现有 protocol → component → Dataset worker，ViewModel 以 replay UUID/source origin 丢弃陈旧事件；
- UI 增加历史回放速度、开始、暂停/恢复、停止和状态计数；回放活动时禁用 live 连接/发送；
- 行读取上限为 `MAX_REPLAY_LINE_BYTES + 1`，并受本次 `max_bytes` 剩余预算限制，避免无界 `readline()`；
- `DatasetBatchEvent` 携带有效 config 快照，避免重配置后的旧 dataset event 污染新窗口。

## 复核发现与处理

1. 初版 worker 使用无参数 `readline()`；独立复核后改为按剩余字节预算和 2 MiB 行上限读取，超长行终止当前回放并保留错误/计数。
2. 初版 `records_read` 上限引用遗漏；已改为使用 `ReplayOptions.max_records`，同时保留全局硬上限校验。
3. 无效记录、TX、非目标 transport 和非单调时间必须区分；当前分别计入 invalid/skipped，历史 RX 不伪装为 live source。
4. protocol/component/Dataset 是异步 worker；回放只重试 protocol offer 的背压，不绕过队列，也不在 UI 线程做解析。

## 验证证据

已运行：

- `uv lock --check`：通过；
- `uv run --locked ruff format --check --no-cache src`：通过，42 files；
- `uv run --locked ruff check --no-cache src`：通过；
- `uv run --locked python -m compileall -q src`：通过；
- `scripts/check.ps1`：通过；
- inline vectors：严格 JSONL、RX-only、TX/UDP skip、坏 base64、非单调时间、historical source、
  protocol/component/dataset 端到端数据：通过；
- inline control vectors：speed、pause/resume 冻结时钟、stop、protocol backpressure retry：通过；
- inline boundary vector：超长行读取受限于 `MAX_REPLAY_LINE_BYTES + 1`，无整行无界载入：通过；
- Qt `QT_QPA_PLATFORM=offscreen`：窗口创建、触发 replay、EOF、窗口关闭和 replay/protocol/component/dataset/recorder
  shutdown：通过；有已知 `QFontDatabase` 缺少字体目录提示，不影响退出；
- `scripts/package.ps1 -Mode onedir`：通过；实际 GUI startup/WM_CLOSE/exit：通过；
- `scripts/package.ps1 -Mode onefile`：通过；实际 bootstrap/GUI child startup/WM_CLOSE/exit：通过；
- 默认 onedir 文件名扫描中的 Bleak/WinRT/Bluetooth/SEGGER/J-Link/probe-rs 匹配数：`0`；
- 回放变更后最终产物：

| 产物 | 大小 | SHA-256 |
|---|---:|---|
| `dist/SerialForge/SerialForge.exe` | 2,984,834 B | `518B1830D6D054CD3BE7E52CC830CC6158CBA24231985F57CF9361F6F3764DA2` |
| `dist/SerialForge.exe` | 47,479,340 B | `1FA6A3889A3D1D56AFD7EB27BABE2A39DFF1657CD3B807D70056953D93429FF7` |

验证结束时没有残留由本轮启动的 `SerialForge` EXE 进程。

## 未运行/未宣称

- 未运行真实 UART/TCP 长时间吞吐、损坏文件压力、干净 Windows、代码签名和第三方许可证发行审查；
- 未运行真实 BLE 扫描/配对/通知、J-Link/目标板和任何驱动/探针操作；J-Link RTT 继续保持最后能力；
- 曲线视图、回放筛选、时间轴缩放和回放导出未纳入 M5c；
- 未创建或修改单元测试、mock、fixture、test harness 或其他 test-only asset；
- 当前任务没有嵌入式 C/C++、固件、MCU、BSP/HAL/RTOS 或硬件源代码变更，因此没有新的厂商固件要求适用性声明。

## 简化评估

通过一个 `ReplayPipelinePort` 和 immutable replay DTO 复用既有 parser/component/Dataset pipeline，
没有引入数据库、图表库、插件运行时、动态代码执行或第二套协议 codec。历史 source gate、generation、
有界文件读取和有界队列使回放错误不会污染实时会话；后续曲线只需消费已有 DatasetBatchEvent。
