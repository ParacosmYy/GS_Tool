# ADR 0017：M5i MAVLink v1/v2 已分帧 RX validator

## 状态

已接受；M5i 只完成已分帧 packet 的结构/CRC 校验与 bounded component projection。真实 UART/飞控、
完整 dialect mapping、签名密钥认证、stream resynchronisation 和 TX 仍未验收。

## 背景

MAVLink v1/v2 的 packet 头部、payload 长度、v2 flags、message id、wire CRC 和可选 signature
有明确的版本差异。当前 SerialForge 的通用 parser 只能从 UART/TCP Client read chunk 观察字节，
不能在没有专用边界契约时把一次 read 可靠地解释为一个 MAVLink packet；同时，CRC_EXTRA 依赖
dialect/message-definition 映射，不能由通用 codec 猜测。把它直接塞进通用 protocol preset 会混淆
stream boundary、协议校验和方言字段语义。

## 来源适用性

- [MAVLink overview](https://mavlink.io/en/about/overview.html)：用于 v1/v2、packet/CRC/dialect
  的公开协议概览；
- [MAVLink packet serialization](https://mavlink.io/en/guide/serialization.html)：用于 v1/v2 header、
  payload/packet 上限、CRC 覆盖范围、little-endian CRC 和 v2 signature 长度；
- [MAVLink message signing](https://mavlink.io/en/guide/message_signing.html)：用于 signed flag、
  13 B signature 和“识别不等于认证”的边界；
- [MAVLink common.xml](https://raw.githubusercontent.com/mavlink/mavlink/master/message_definitions/v1.0/common.xml)：
  作为官方 common dialect message-definition 来源。示例 profile 的 message id 0/CRC_EXTRA 50
  使用官方 common 参考，但 `master` 不是生产锁定 revision；
- 以上是 MAVLink 官方协议/实现参考，不是具体飞控或 MCU 制造商要求；本 checkout 没有 C/C++、固件、
  MCU、BSP/HAL/RTOS 或目标硬件改动，不作嵌入式认证/合规声明。

## 决策

1. 新增无 Qt、无 transport、无动态插件的 `domain.mavlink`，只接受一个已分帧 payload，返回不可变
   `MavlinkFrame`；v1/v2 结构和 packet 长度在一个有界 validator 内完成；
2. CRC 使用 MAVLink 的 CRC-16/MCRF4XX 算法，并要求 schema v2 profile 显式声明有限的
   `message_id -> CRC_EXTRA` mapping。mapping 缺失时状态是 `UNVERIFIED`，绝不把结构解析冒充 wire
   integrity；
3. v2 signed packet 保留 13 B signature；即使 wire CRC 正确，也因没有密钥/认证过程而保持
   `UNVERIFIED`。未知 incompat flag、header/length/packet 错误和 bad CRC 仍成为可见 frame status；
4. schema v2 增加 `mavlink` component codec，默认只展示有限 packet metadata、payload Hex 和
   signature Hex。`dialect`、`mapping_source`、`mapping_revision` 都是 profile 数据；字段错误阻止
   非 `VALID` packet 进入 Dataset；raw terminal/recorder 仍保留完整 wire bytes；
5. 不增加 MAVLink runtime、dialect generator 或 crypto runtime，不增加 protocol preset，不实现
   stream resync、完整 message payload field generation、signature key management、TX、request/
   response、飞控命令或自动重试。未来这些能力必须通过独立 boundary/port 接入。

## 结果与风险

该切片只增加一个纯领域 validator、一个静态 codec 分支、配置加载/导出和 UART-only scope guard，
不新增运行时依赖、transport 线程、Qt 入口或 RTT 路径。它适合历史回放、显式 boundary shim 和已经
由设备/上游分帧的完整 packet；普通 serial read 可能拆分或合并多个 MAVLink packet，因此当前不能
宣称实时 MAVLink stream 支持。CRC_EXTRA mapping 若未固定方言 revision，结果只能保持 unverified；
signature 字节可审计但不提供真实性或防重放保证。

## 验证

已用 inline vector 验证 CRC-16/MCRF4XX、v1/v2 valid packet、缺失 mapping、bad CRC、signed v2
signature、未知 incompat flag、sysid/compid 0、截断/超长/多余字节、codec dump/load、row status/
fields、非 `VALID` 字段 error 和 UART-only scope。显式 boundary shim、Qt offscreen、`scripts/check.ps1`、
onedir/onefile startup/close、archive entry、warning scan 和最终包 hash 仍须在本轮最终代码后记录。
未连接真实 UART、飞控或 J-Link，未验证 stream resync、完整方言、签名密钥、TX 和飞控 message semantics。
