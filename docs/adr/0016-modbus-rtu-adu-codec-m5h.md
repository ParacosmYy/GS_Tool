# ADR 0016：M5h Modbus RTU 已分帧 ADU RX codec

## 状态

已接受；M5h 只完成已分帧 ADU 的 RX 校验与 bounded component projection，真实 UART、RTU 时序分帧、
完整 function-code codec 和主从事务仍未验收。

## 背景

Modbus RTU 的 wire frame 不是普通“payload 尾部 CRC”这么简单：serial-line 规范定义了 address、
function、data 和两字节 CRC，CRC 低字节先传；RTU 的帧边界依赖字符间/帧间静默时间。当前 SerialForge
的 UART/TCP Client parser 只能观察任意 read chunk，且通用 checksum parser 会把派生 payload 的
尾部校验字段消费掉。若直接把 Modbus 塞进通用 preset，会同时破坏边界和 CRC 归属。

## 来源适用性

- [Modbus Organization Serial Line V1.02](https://www.modbus.org/file/secure/modbusoverserial.pdf)，
  2006-12-20，§2.3、§2.5.1.1、§2.5.1.2：用于 ADU 布局、256 B 上限、t1.5/t3.5 边界和 CRC 线序；
- [Modbus Application Protocol V1.1b3](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)：
  用于 PDU/function/exception 的语义边界；
- 以上是 Modbus Organization 的协议标准参考，不是具体 MCU/设备制造商要求；本 checkout 没有
  C/C++、固件、MCU、BSP/HAL/RTOS 或目标硬件改动，不作嵌入式认证/合规声明。

## 决策

1. 新增无 Qt、无 transport、无动态插件的 `domain.modbus`，只接受一个已分帧 payload，返回不可变
   `ModbusRtuFrame`；CRC 使用标准初始化/多项式，接收线序按低字节在前；
2. 地址 0–247 可进入 validator，248–255 是格式错误；function `0x00` 与 `0x80` 是基本格式错误，
   其他 function byte（包括未知码和高位异常码）只展示，不判定完整业务语义；
3. schema v2 增加 `modbus_rtu` component codec，默认或显式声明有限字段 source：address、function、
   exception、data length/Hex、exception code、received/calculated CRC；不执行表达式或脚本；
4. Modbus 结果复用现有 `ComponentCodecRouter`、bounded worker、table/filter/CSV 和 Dataset；坏
   Modbus 状态同时成为 row/status 和字段 error，使坏 CRC 不进入 Dataset 计算；raw terminal/recorder
   仍保留完整 wire bytes；
5. 不增加 Modbus protocol preset，不在通用 parser 中猜测 t1.5/t3.5，不实现自动重同步、request/
   response 关联、master/slave transaction、ASCII、完整 function-code/PDU fields、寄存器映射、TX、
   ACK 或重试。未来 timing-aware framer 必须拥有独立的边界契约。

## 结果与风险

该切片只增加一个纯领域 validator、一个静态 codec 分支和配置加载/导出，不新增运行时依赖、transport
线程、Qt 入口或 RTT 路径。它适合历史回放、显式边界 shim 和已经由设备/上游分帧的完整 ADU；普通
Windows serial read 可能拆分或合并多个 RTU ADU，因此当前不能宣称实时 Modbus RTU stream 支持。
未知 function、寄存器地址范围、PDU 长度合法性和 request/response 角色仍需设备配置及后续独立 profile。

## 验证

已用 inline vector 验证标准 request `01 03 00 00 00 0A C5 CD`、异常 response `01 83 02`、坏 CRC、
保留地址、非法 function、截断/超限、codec dump/load、row status/fields 和 Dataset field-error 边界；
显式 U16 boundary shim、Qt offscreen、`scripts/check.ps1`、onedir/onefile startup/close、archive
entry、warning scan 和最终包 hash 均通过。未连接真实 UART、Modbus 设备或 J-Link，未验证 t1.5/t3.5
时序、噪声重同步、寄存器映射和完整事务。
