# ADR 0006：M4a 有界协议解析与组件预览

- 状态：已接受
- 日期：2026-08-09
- 范围：UART/TCP Client RX 的 Raw/Line/Delimiter/Length framing、固定 checksum preset 和通用帧预览

## 决策

协议解析不放进 UART/TCP/BLE 适配器，也不放进 Qt 槽函数。transport 只发布已有 raw ingress
事件；`ProtocolIngressUnit` 携带 session、transport、peer/peer_id、channel、direction、边界类型和
payload；`ProtocolPipelineWorker` 通过有界队列交给纯 domain decoder，并按 source key 保存
独立 partial state。

当前组件入口只接收 UART/TCP Client 的 RX，UDP、TCP Server 多 peer 和 BLE 仍只显示 raw
终端并写入原始记录。这让 M4a 可先验证 stream partial frame，同时不给尚未冻结的组件语义
制造隐式行为；source-aware pipeline 为后续扩展保留稳定端口。

## 帧与校验语义

- Raw 每个输入片段为一项，不启用 checksum；
- Line 使用 LF，CRLF 的 CR 从 payload 移除；
- Delimiter 分隔符由非空 Hex 配置，不进入 payload，空帧可见；
- Length prefix 使用 1/2/4 字节、little/big endian，长度只表示 payload，最大 65536 B；
- XOR-8、CRC16/Modbus、CRC32/IEEE 的 checksum 固定在帧尾，字节序可选；
- checksum 失败、长度失败、超限和 disconnect incomplete 都生成带状态的 frame，不关闭连接；
- parser 只生成组件预览事件，队列/事件背压不会截断或改变 RawRecord。

M4a 不支持任意多项式、整帧长度基准、自动重同步、TX 自动编码、JSON/TLV/field codec、
profile 保存迁移、图表或回放导出；这些进入后续 M4b/M5 评审。

## 关闭与重置

切换连接、配置或点击“重置解析”会清理 parser partial state 和统计；不会断开 transport、
改变发送内容、补 CRLF 或修改已有 JSONL raw schema。parser worker 在应用退出时使用有界
shutdown，UI 只消费不可变 `ProtocolFramesDecodedEvent` 和 backpressure 状态。

## 验收证据

无硬件条件下执行 domain/manual vectors：跨读取 Line、Delimiter、Length、XOR/CRC、source
隔离和 datagram incomplete；执行 protocol worker 事件向量、Qt offscreen 启动/关闭、静态
检查和 compile。未执行真实 UART/TCP 吞吐、长时间压力、clean Windows、签名和发行法务门。
