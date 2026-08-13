# ADR 0013：M5e 有界声明式批量命令

状态：已接受（2026-08-09）  
范围：UART、TCP Client、TCP Server 单 Peer、UDP 单播和 BLE GATT 的显式有限发送序列。  
不包含：J-Link RTT 批量发送；RTT 继续作为最后能力，当前只保留单条发送。

## 背景

串口和网络调试经常需要重复执行“初始化 → 查询 → 配置”这样的固定命令序列。
现有 `CommandEntry` 只表示一条文本/Hex 命令，快捷命令适合单步但不能表达有限顺序。
宏功能必须保持可解释、有界，并继续复用已有 `SessionPort` 和各传输的 typed write。

## 决策

- 新增 `CommandBatch` / `CommandBatchStep` immutable DTO；宏只保存静态 payload、模式、CRLF 和固定延时；
- 宏目录只在当前进程内存中保存，最多 16 个；每个宏 1–32 步；单步最终 payload（含 CRLF）最多 512 B；总 payload 最多 16 KiB；
- 步骤间延时为 0–2,000 ms，总延时最多 180 s；不支持循环、条件、变量、嵌套、脚本、外部命令、响应等待或 ACK 判断；
- 运行前快照 session、transport config、TCP `PeerId` 或 BLE characteristic/write mode，并先构造全部 typed write；首步入队前的校验失败不发送任何字节；
- application 的 `CommandBatchService` 使用独立后台线程逐步调用现有 `SessionPort.send()`，延时使用可取消的 `Event.wait()`；
- “已提交/已完成”只表示本地 session 出站队列接受，不表示线缆完成、对端处理成功或收到应用层 ACK；已接受的写入不能撤回；
- 任一步 `SessionPort.send()` 失败、用户停止或 session 断开都会阻止后续步骤；不自动重试、不自动切换 Peer、不广播；
- 宏步骤沿用现有历史和 raw recorder 路径：每个已接受步骤进入发送历史，真实发送仍由 session worker 记录；回放仍 RX-only；
- 执行期间 UI 禁止普通单条发送、快捷命令、编辑和删除，避免人为交错；窗口关闭顺序为停止批量命令，再关闭 session；
- presentation 提供紧凑编辑器和最多 32 行步骤结果；RTT 明确显示为后续能力，不因本轮引入驱动、SDK 或 SEGGER DLL。

## 未选择的方案

- 不把步骤列表扩展成 `SessionPort.send(list[...])`：这会把批量语义污染每个 transport worker，并无法提供原子性；
- 不在 Qt 槽函数中 `sleep` 或运行设备 I/O：调度属于 application worker；
- 不用 `eval`、动态导入、脚本文件、定时任务库或插件执行宏；
- 不宣称逐步线缆完成：现有 session API 没有 operation/correlation ID，本轮不扩张所有传输的完成事件链；
- 不做跨重启 JSON 文件导入/导出：需要独立版本化 schema 和恢复策略，留给后续 ADR。

## 边界与风险

- `CommandEntry` 保持单条历史/快捷命令 DTO，不携带 session、Peer、BLE 状态或子步骤；
- UDP 保留每步 datagram 边界，但实际发送仍由既有 adapter 检查远端和配置的最大 datagram；
- BLE 不自动分片；with-response 和 characteristic 的 without-response 能力仍由既有模型/adapter 检查；
- 由于 `SessionPort.send()` 是入队级 API，宏与低层 session FIFO 之间不提供跨调用原子性；UI 只保证宏运行时不主动插入单条发送；
- RTT 驱动/工具未安装不阻塞本轮，RTT 仍按用户要求最后接入。

## 复核角色

| 角色 | 子代理 run |
|---|---|
| 产品 | `019fe484-ff4e-7bf3-a16e-d2cef7605539` |
| 架构 | `019fe484-ff87-7bf2-872f-a9c60f8028c5` |
| UI 设计 | `019fe484-ffc7-7ec0-9e6f-1b664668f9a6` |
| 开发 | `019fe485-0014-7dc0-8292-ac3916eb6d59` |
| 验证 | `019fe485-004e-7d62-9da4-dff1ccba87d1` |
| 打包/流程 | `019fe485-008c-7572-ac4d-6dc246af9412` |

