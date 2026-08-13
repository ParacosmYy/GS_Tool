# ADR 0022：实时协议流边界（M5m）

状态：已接受；后端代码切片完成，UI 配置入口、真实串口时序和完整验收后置。

## 背景

M5h 和 M5i 的 Modbus RTU、MAVLink validator 只接受已经分好的完整 payload。把一次
UART/TCP read 当成完整协议帧会造成半包、粘包和噪声误判，因此需要在协议 codec 之前增加
独立的、按 source 隔离的 stream boundary。该边界不能改变 raw terminal、JSONL recorder 或
transport 的事实路径。

Modbus RTU 的边界依赖 t1.5/t3.5 静默间隔。当前桌面 ingress 只能可靠获得 host read 之间的
monotonic gap，不能把它当作每个字节在线缆上的时间戳；因此 timing DTO 必须携带质量来源。
MAVLink 的 packet 长度可以用于结构提取，但 CRC_EXTRA 和签名认证仍依赖显式 profile/密钥。

## 决策

- 在 `domain.timing` 定义不可变 `GapObservation` 和 `ModbusRtuTiming`；质量明确区分
  `HOST_READ_GAP`、预留的 `DEVICE_TIMESTAMP` 和无时序事实的 `NONE`；
- `ProtocolIngressUnit` 携带 parser-only timing，不向 transport、recorder、component 或
  Dataset 泄漏设备身份或 replay session 语义；实时 session 由 application 计算 host read gap；
- `MavlinkStreamDecoder` 独立于 `MavlinkComponentCodec`：按 FE/FD、header length、signed
  signature length 和 280 B 上限提取 packet，丢弃噪声并保留有限 resync/drop 统计；结构完整但
  没有 CRC_EXTRA/profile 时发布 `UNVERIFIED`，由显式 component profile 再升级为 `VALID` 或
  发布 CRC 错误；
- `ModbusRtuStreamDecoder` 只在 `GapObservation` 质量为 `HOST_READ_GAP` 或
  `DEVICE_TIMESTAMP` 时应用 t1.5/t3.5。t3.5 结束候选 ADU，t1.5 与 t3.5 之间发布
  `INCOMPLETE` 并清理候选；没有 timing quality 的普通 `feed(bytes)` 只追加数据，不能凭
  read chunk 分帧；
- protocol worker 的 ingress queue 采用 source continuity epoch。队列丢弃后，下一段数据先
  结束旧 partial 并 reset，再进入新 epoch，防止被丢掉的字节跨 gap 拼成合法帧；
- `ProtocolStats` 暴露 incomplete、gap boundary、resync 和 dropped bytes；raw 数据路径和
  会话发送语义不因协议边界改变。

## 简化与边界

本轮不增加计时线程、每字节 UART hook、协议脚本、自动重连、请求/响应事务或第三方协议库。
host read gap 只是一种诊断性边界证据；只有驱动/设备明确提供每字节或硬件 timestamp 时，
后续切片才能新增 `DEVICE_TIMESTAMP` 适配。MAVLink resync 是有界结构启发式，不是完整 dialect
语义校验；签名仍未认证。Modbus ADU codec 继续只消费完整 payload，不重复消费 CRC。

当前 MainWindow 已把 `MAVLINK_STREAM`、`MODBUS_RTU_TIMED`、显式 preset application、当前
UART 派生的 `modbus_timing` 和新增统计接入编辑控件；Modbus host-gap 仍显示为诊断证据。该项
保持与二次元动态 UI 分离，后者作为下一轮 presentation-only 切片处理，避免业务语义和视觉
重构互相耦合。GUI offscreen、真实 per-byte timing、事务/方言/签名和正式发行仍是后置验收门。

## 验证

已授权且非破坏性的当前证据：

1. `compileall`、Ruff check 和 Ruff format check 通过；
2. 临时进程内 MAVLink bytes 向量覆盖噪声、结构完整但无 profile 的 `UNVERIFIED`、坏长度
   启发式重同步和 profile CRC_EXTRA 升级；
3. 临时进程内 Modbus bytes 向量覆盖 t3.5 host-gap 完整 ADU、无 timing gap 的合并和
   t1.5～t3.5 的 `INCOMPLETE`；
4. 队列丢弃 continuity barrier 通过静态审查和代码路径检查；
5. 未启动 GUI/EXE，未连接、写入、刷写或操作 UART、BLE、Wi-Fi、J-Link 目标硬件。

依据与适用性：Modbus t1.5/t3.5 依据公开的一方协议资料
[Modbus Serial Line V1.02](https://www.modbus.org/file/secure/modbusoverserial.pdf)，
适用范围是 Modbus Serial Line RTU framing；MAVLink 继续沿用项目已记录的官方协议/实现参考，
不是某个飞控厂商的制造商要求。本项目没有修改嵌入式 C/C++ 或固件，因此嵌入式企业流程对
源码适用性为 N/A，不能据此宣称任何固件或认证合规性。
