# M5h 六角色复核记录：Modbus RTU 已分帧 ADU RX validator/profile

日期：2026-08-09  
范围：独立 Modbus RTU ADU 校验、schema v2 component codec、UART 作用域、raw/Dataset 边界；不含
J-Link RTT、不含真实硬件、不含 t1.5/t3.5 timing framer 或完整主从事务。  
源码写入者：父代理；六角色子代理均为只读复核，无子代理直接修改当前 checkout。

## 六角色记录

| 角色 | 子代理 run | 结论 | 关键建议/父代理动作 |
|---|---|---|---|
| 产品 | `019fe4b8-becc-7941-b052-046bf1282f9b` | 条件通过 | 冻结为已分帧 ADU RX，禁止宣传实时 RTU/完整事务；已整合 |
| 架构 | `019fe4b8-bf04-77a3-a8b6-825837ce9fdd` | 条件通过 | 独立 `domain/modbus.py`，复用静态 component router/worker，不新增 transport/event；已整合 |
| UI 设计 | `019fe4b8-bf40-74e2-9eb3-e399037917a0` | 有条件支持 | 复用现有 Profile/表格，Modbus 仅 UART，raw-only/已分帧边界可见；已整合 |
| 开发 | `019fe4b8-bf80-73e1-a7d4-f8be24e2ffaf` | 条件通过 | CRC 归属单一、返回基础 function/exception code 和分层错误；已整合 |
| 验证 | `019fe4b8-bfbb-7600-8dfc-9017bf3e47f7` | revise → 条件通过 | 正/坏 CRC、地址/function、异常、截断、超限、显式边界 shim；真实 RTU 后置 |
| 打包/流程 | `019fe4b8-bffa-7943-929c-20c423337366` | 条件通过 | 无新依赖；补 ADR/示例 profile/六角色记录，最终重建双模式包；已整合 |

六个子代理均已完成并关闭；父代理负责代码整合、最终 diff review 和验证。

## 公共来源适用性

- [Modbus Organization Serial Line V1.02](https://www.modbus.org/file/secure/modbusoverserial.pdf)，
  2006-12-20，§2.3、§2.5.1.1、§2.5.1.2：用于 address/function/data/CRC ADU、256 B 上限、
  t1.5/t3.5 timing 边界、CRC 初始化/多项式和低字节先传；
- [Modbus Application Protocol V1.1b3](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)：
  用于 PDU、function code、exception bit/code 的语义边界；本轮只展示，不实现完整业务 codec；
- [Modbus 官方规格索引](https://www.modbus.org/modbus-specifications)：用于确认官方 serial-line
  文档状态/适用范围；以上均为协议标准工程参考，不是特定 MCU/设备制造商要求。

当前 checkout 是 Python/PySide6 Windows 桌面应用，没有 C/C++、MCU、BSP/HAL、RTOS、固件、编译器
或目标硬件改动；嵌入式厂商要求、固件简化和硬件 R&D 验证本轮 N/A。未作 MISRA、ISO 26262、ASIL、
ASPICE、Modbus 认证或设备互操作认证声明。六个角色均记录了 assurance marker。

## 变更摘要

- 新增 `src/serialforge/domain/modbus.py`：bounded immutable `ModbusRtuFrame`、标准 CRC16、地址/
  function 基本格式、截断/超限/坏 checksum 状态；明确不推断 t1.5/t3.5；
- `src/serialforge/domain/codecs.py` 新增 schema v2 `modbus_rtu`、有限 `name/source` 字段和显式
  router 分支；默认字段包含 address、raw/base function、exception、data length/Hex、exception
  code、received/calculated CRC；
- 坏 Modbus 状态同时作用于 component row status/error 和字段 error，Dataset 会保留错误而不计算；
  raw terminal/recorder 仍是完整 wire truth；
- `viewmodels.py`/`main_window.py` 复用现有 Profile 入口，Modbus codec 仅允许当前 UART；TCP/UDP/
  BLE/RTT 继续 raw-only 或拒绝该 codec；
- `profiles/modbus-rtu-rx-adu.json` 是可直接加载的用户示例，不是测试专用资产；新增 ADR、README、
  roadmap、protocol、constraints、workflow、architecture、research 和 dependencies 记录；
- 无新增运行时依赖、端口、线程、transport 分支、J-Link SDK/DLL 或 RTT 改动。

## 独立复核与简化评估

复核确认当前 UART/TCP Client 只提供任意 stream read chunk，通用 framing checksum 可能先消费并
移除 CRC；因此 M5h 不新增 Modbus preset，也不让 codec 伪装 stream framer。纯 validator 只拥有
ADU/CRC/基础 field projection；已有 component worker、过滤、CSV、Dataset、raw recorder、UI 表格
和关闭生命周期复用。这样保持 domain 高内聚、transport/codec 低耦合，未来 timing-aware framer 或
完整 function-code profile 可以作为独立切片接入。

## 验证证据

已运行：

- `uv lock --check`：通过；
- `scripts/check.ps1`：通过（locked sync、Ruff、compileall）；
- `uv run --locked ruff format --check --no-cache src`、`ruff check --no-cache src` 与 `compileall`：通过；
- inline domain vectors：`01 03 00 00 00 0A C5 CD`、异常 `01 83 02`、坏 CRC、保留地址、非法
  function、截断、超限、CRC 低字节顺序、codec dump/load：通过；
- inline Dataset vector：坏 Modbus row 的 field error 使 dataset value 保持 `None`：通过；
- explicit-boundary protocol/component shim：U16 length 仅作为边界载体，拆分两条完整 ADU：通过；
  该结果明确不等价于 RTU t3.5 timing；
- Qt `QT_QPA_PLATFORM=offscreen`：示例 profile 加载、UART 接受、TCP/UDP scope 拒绝/raw-only 和
  应用/worker 关闭：通过；已知 PySide6 font-directory 提示不影响退出；
- `scripts/package.ps1 -Mode onedir`：通过；实际 GUI startup/WM_CLOSE/exit 通过，onedir PID `80888`；
- `scripts/package.ps1 -Mode onefile`：通过；实际 bootstrap `86292`、GUI `63228` startup/WM_CLOSE/exit 通过；
- 默认 onedir 文件名扫描 `Bleak|WinRT|Bluetooth|SEGGER|JLink|probe-rs`：`0` 匹配；PyInstaller
  Analysis/PYZ archive 包含 `serialforge.domain.modbus` 和 `serialforge.domain.codecs`；warning 文件
  仅列出预期的 Windows/可选 BLE/跨平台条件导入；没有残留 SerialForge 进程；
- 最终产物：`dist/SerialForge/SerialForge.exe` 3,040,134 B，SHA-256
  `792A32544BA3DF7D101BABF1BD0F97B6CD01FC04A595BFBE834369F89DA7DF3D`；`dist/SerialForge.exe`
  47,534,931 B，SHA-256 `BAC50D98C328B0B72B9E55A4B4E8EFD27266F11CA472204D9C6DFDECB9BA5717`。

未运行/未授权：真实 UART/Modbus 设备、RS-485 电气层、t1.5/t3.5 精确时序、噪声/丢字节重同步、
request/response 匹配、主从事务、寄存器映射、完整 function-code/PDU codec、Modbus ASCII、BLE/
TCP/UDP/RTT Modbus 语义、J-Link 驱动/探针/目标板和干净 Windows 安装验证。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
