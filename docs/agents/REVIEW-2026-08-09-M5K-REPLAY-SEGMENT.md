# M5k Replay session segment 六角色记录

日期：2026-08-09  
范围：隔离同一 recorder JSONL 内相邻原始 session 的 protocol partial buffer；不扩展实时
Modbus/MAVLink stream boundary、transport、BLE 或 J-Link RTT。  
父代理是唯一源码写入者；六角色只读审查；未创建/修改/运行测试专用资产。

## 六角色结果

| 角色 | 子代理 | 结果 | 关键结论 |
|---|---|---|---|
| 产品 | `019fe4e9-df68-7c43-ad55-3ce43e2ebf7f` | revise | 实时协议 timing boundary 是后续 P0；M5k replay segment 是当前独立 P1，必须按原始 session 隔离 partial tail。 |
| 架构 | `019fe4e9-dfa7-7b72-90a9-b1c937e60cca` | revise | generation/source 不能解决同一 JSONL 内的 session boundary；建议 parser-only segment metadata 和 FIFO 有序生命周期。 |
| UI 设计 | `019fe4e9-dfe9-78c3-ba0c-a68024ccd49a` | revise | 本轮不扩大 presentation；segment 显示、local error 和 transport context 另开切片。 |
| 开发 | `019fe4e9-e030-7d23-8e96-3af23982373e` | revise | 最小源码集合为 `domain/protocols.py`、`application/replay.py`、`application/protocols.py`，端口只补契约说明。 |
| 验证 | `019fe4e9-e074-73b0-97b9-5ef23c6dbf8f` | revise | 需用 inline worker 向量验证 incomplete tail、新 segment 不拼接、FIFO 不丢已接受 ingress，再复跑静态/Qt/包门。 |
| 打包/流程 | `019fe4e9-e0b2-7573-9e32-b7e8b8f9929c` | revise | 默认依赖隔离正确，但正式发布仍缺 PE metadata、变体隔离、manifest/hash 和实际许可证材料。 |

六个子代理均使用 `luna_max`，已完成并关闭；没有任何子代理修改 checkout。

## 独立复核与简化

- `domain.replay.ReplayRecordCodec` 已保留原始 `RawRecord.session_id`，但旧实现只用 replay UUID
  构造 `ProtocolSource`；同 source 的 streaming decoder 会跨原始 session 保留 partial buffer；
- `ProtocolIngressUnit.segment_id` 只属于 parser lifecycle，默认 `None` 兼容 live ingress；不把
  replay segment 复制到 `ProtocolSource`、events、component、Dataset 或 UI；
- replay worker 只标注原始 session；protocol worker 在 FIFO 出队点完成 `finish → reset → feed`，
  因而不让 replay producer 线程直接修改 parser generation/队列；
- segment map 随 configure/reset 清理；连续重复 UUID 通过相邻 segment 比较而不是全局 UUID 集合，
  保持内存有界；
- 本轮不实现实时 Modbus t1.5/t3.5、MAVLink resync、local error、发布许可、真实设备或 RTT。

## 验证记录

父代理整合后的证据：

- inline `ProtocolPipelineWorker` vectors 输出
  `M5k replay segment vectors passed 4 ['valid', 'incomplete', 'valid', 'valid']`：同 segment 跨
  chunk、session 变化产生 `INCOMPLETE`、新 segment 不拼接、连续重复 UUID 重新隔离；
- `uv lock --check`、Ruff format/check、compileall 和 `scripts/check.ps1`：通过；
- `python -B` 动态导入当前 `serialforge` 包及其 49 个子模块：共 50 个模块通过；
- `QT_QPA_PLATFORM=offscreen` 创建/配置/关闭 Qt 应用：`M5k Qt offscreen close passed 0`，
  `SerialForge` 残留进程数为 0；
- onedir/onefile 当前源码启动并通过 WM_CLOSE：onedir PID 44712、onefile launcher PID 59364 /
  GUI PID 99960，GUI 均正常退出；
- onedir `dist/SerialForge/SerialForge.exe`：3,055,988 bytes，SHA-256
  `D2D741476DF3B0C6C4AD46F79F077CEE6F893741BEEC30A9315457712A5A6D25`；onefile
  `dist/SerialForge.exe`：47,551,462 bytes，SHA-256
  `2B2A67F9525F847082D12C8DA7C9E3986D361A7BAC23A548687E6E7536AF1281`；
- onefile archive 包含 `serialforge.domain.events`、`serialforge.domain.protocols`、
  `serialforge.domain.replay`；默认 onedir optional scan 为 0，未发现 `Bleak`、`WinRT`、
  `SEGGER`、`JLink` 或 `probe-rs`；最终残留 `SerialForge` 进程数为 0；
- 未创建测试文件、mock、fixture 或测试 harness；真实 UART/BLE/TCP/UDP/RTT 未运行。

## 未完成与后续门

- P0：真实 USB-UART loopback、参数/拔插/吞吐和 raw JSONL 证据；
- P1：实时 Modbus/MAVLink timing/resync 和 BLE 实机；
- G0：PE version metadata、版本/变体输出隔离、manifest/hash、实际 NOTICE/许可证材料和干净 Windows；
- M6：J-Link 驱动、目标板、RTT Telnet 和授权硬件最后验收。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
