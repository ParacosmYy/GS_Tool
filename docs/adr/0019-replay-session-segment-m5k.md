# ADR 0019：Replay session segment parser boundary（M5k）

状态：已接受；M5k 代码切片，真实设备和正式发行门后置。

## 背景

一个 recorder JSONL 文件可以包含多个原始连接 session。M5c replay 为整个回放运行生成一个
`replay_id`，并把所有历史 ingress 映射到同一个 `ProtocolSource`。protocol pipeline 又按
`ProtocolSource` 保留 streaming decoder，因此当相邻原始 session 使用相同 transport/peer/channel
时，前一个 session 的 partial bytes 可能与后一个 session 拼接。

## 决策

- `ProtocolIngressUnit.segment_id` 是 parser lifecycle metadata，类型为可选 UUID；默认 `None`
  保持 live ingress 和既有调用兼容；
- replay worker 为每条可回放的 RX stream ingress 传入原始 `RawRecord.session_id`，但继续使用
  replay UUID 作为 `ProtocolSource.session_id`，不改变 historical source gate、`ReplayDataEvent`
  或 component/Dataset event；
- `ProtocolPipelineWorker` 在 FIFO 出队点保存 source → segment 映射；发现 segment 变化时，在
  同一 worker lock 内先 `ProtocolPipeline.finish(source)` 发布旧 partial frame 的 `INCOMPLETE`
  结果，再 `reset(source)`，然后 feed 新 segment；
- segment map 随 protocol configure/reset 清理；同一 UUID 在日志中再次出现时，只要中间出现了
  其他 UUID，也按新的连续 segment 处理；
- segment 边界只影响 parser decoder state 和 parser stats。Replay 的 component/Dataset 仍是同一
  run 的 bounded preview；segment 选择器和 UI 分段展示另行设计；
- 不从 replay 线程直接调用 protocol reset，不改变 transport、recorder、队列上限、generation
  语义、RTT 或实时 Modbus/MAVLink timing/resync。

## 为什么不把 segment 放进 ProtocolSource

`ProtocolSource` 是 downstream source identity，必须继续表示同一 replay run 的 historical stream，
否则 ViewModel、component 和 Dataset 都要复制 replay-specific session 分支。`segment_id` 只在
ingress/worker 内表达 parser 生命周期，保持 domain/application 边界的高内聚和低耦合。

## 验证要求

使用临时 inline bytes/事件，不创建测试文件、mock、fixture 或 harness：

1. 同一 segment 的分片 Line/Delimiter ingress 能跨 chunk 形成完整 frame；
2. segment 变化先发布旧 tail 的 `INCOMPLETE`，新 frame 不包含旧 bytes；
3. FIFO 已接受 ingress 不被 producer 线程直接 reset 丢弃；
4. configure/reset 清理 segment map；连续重复 UUID 按连续段重新隔离；
5. Ruff、compileall、`scripts/check.ps1`、Qt offscreen 和现有 onedir/onefile 启动/关闭门通过。

真实 UART、BLE、TCP/UDP acceptance、实时 Modbus/MAVLink stream boundary、J-Link RTT、干净
Windows 和正式许可证仍未由本 ADR 证明。
