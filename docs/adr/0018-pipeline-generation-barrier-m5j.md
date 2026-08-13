# ADR 0018：M5j pipeline generation barrier

## 状态

已接受；M5j 完成 protocol → component → Dataset 的 generation chain 和 stale-event fence。
Replay session segment、真实硬件验收、实时协议 stream boundary、本地 UI error 统一状态和正式
发行许可证仍未完成。

## 背景

三个 worker 已经各自用本地 `_generation` 清空 pending queue，但 generation 没有进入对外事件。
配置/reset 与 worker 解码/发布之间存在窗口：旧 protocol frame 可能在新 component profile 下
被重新处理，旧 component event 也可能在新 Dataset config 下产生样本；ViewModel 原先只比较
session/profile/config，无法阻止后端统计和 retention 被污染。

## 决策

1. `ProtocolFramesDecodedEvent` 和 `ProtocolBackpressureEvent` 携带 protocol `generation`；
   `ProtocolPipelinePort.generation` 暴露当前 root fence；live `StreamDataReceivedEvent.occurred_at`
   原样传入 `ProtocolIngressUnit`，协议事件不再丢失捕获时间；
2. `ComponentFramesDecodedEvent` 携带自身 `generation` 与 `protocol_generation`，component worker
   在 `offer()` 入队前拒绝低于 `protocol_generation` fence 的事件，并在 commit/publish 前检查自身
   generation；component port 的 configure/reset 可更新 upstream fence；
3. `DatasetBatchEvent` 携带自身 `generation`、`component_generation` 和 `protocol_generation`；
   Dataset worker 在入队前拒绝过期上游事件，提交前检查自身 generation；Dataset port 的 configure/
   reset 可更新两个 upstream fence；
4. ViewModel 在每次 protocol/component/Dataset 配置或 reset 后同步 worker generation，消费时要求
   完整 generation chain 和现有 source/profile/config 都匹配；旧事件即使已经进入 EventBus，也不能
   更新 rows、stats、samples 或曲线输入；
5. generation 只属于 application pipeline/event DTO，不修改 transport、raw recorder、队列大小、
   协议 codec 或 RTT；不引入全局可变状态、动态插件或新运行时依赖。

## 不在本 ADR

- Replay 原始 recorder session 变化时的 segment reset；
- MAVLink stream resynchronisation、Modbus t1.5/t3.5 timing framer；
- 统一 MainWindow local error 状态清除；
- 真实 USB-UART、BLE、J-Link RTT、干净 Windows、发行签名和许可证放行。

## 验证

使用临时 inline bytes/事件和实际 worker 验证：protocol capture time 保留、root generation 从 0
变化、旧 protocol event 被 component upstream fence 拒绝、旧 component event 被 Dataset upstream
fence 拒绝、fresh generation chain 可通过；Ruff、compileall、现有检查脚本和 Qt API/offscreen 需在
最终源码后复跑。没有创建或运行测试专用文件、mock、fixture 或 harness。

## 简化与风险

本设计保留三段 worker 和现有 domain/application/presentation 依赖方向，只增加不可变事件元数据
和显式 fence；没有把 generation 逻辑塞进 transport 或 codec。仍需后续处理 Replay session boundary，
否则同一 replay source 下相邻原始 session 可能保留 partial stream 状态；该风险不能由 generation
字段隐式掩盖。
