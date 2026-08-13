# M5i MAVLink v1/v2 RX codec 六角色记录

日期：2026-08-09  
范围：已分帧 MAVLink v1/v2 RX validator、schema v2 `mavlink` component codec、UART-only
scope guard、profile 示例和最终 Windows 打包。  
写入者：父代理；六个角色均为只读审查，未创建/修改/运行测试专用资产。

## 角色结果

| 角色 | 子代理 | 结果 | 独立结论 |
|---|---|---|---|
| 产品/保证 | `019fe4c7-23e9-72e1-816a-50de8c07eebd` | pass with boundary | 只做窄范围已分帧 validator/profile；缺少 CRC_EXTRA 必须未验证；不扩展 RTT、TX 或实时边界。 |
| 架构 | `019fe4c7-2423-77e2-b193-f02e38172428` | pass with boundary | 新增独立 `domain.mavlink`，复用静态 component router/worker；不修改通用 stream framer。 |
| UI 设计 | `019fe4c7-2463-7fd3-b645-a4d7bba73ec1` | revise then pass | 要求 UART-only、显式 mapping/revision、`UNVERIFIED` 状态和 signed 未认证文案；均已落实。 |
| 开发 | `019fe4c7-24ab-7a40-bc61-76f80120cff1` | pass with boundary | CRC-16/MCRF4XX、header-dependent length、CRC_EXTRA 和 signed 长度可窄范围实现；不做完整方言/签名认证。 |
| 验证 | `019fe4c7-24ee-71e0-ae95-f3e445fe7ed8` | conditional pass | 要求官方映射来源/revision 可追溯、inline/offscreen/package vectors；本记录已补齐。 |
| 打包/流程 | `019fe4c7-252d-7f42-a3f4-d4a072382ebb` | pass | 无新增 runtime dependency/hidden import；默认包保持 BLE/SEGGER 可选实现隔离。 |

六个角色均已结束并关闭；每个角色的保证结论均明确：本 checkout 是 Python/PySide6 Windows
应用，无 C/C++、MCU、固件、BSP/HAL、RTOS、厂商 SDK 或硬件改动，因此嵌入式厂商要求适用性为
N/A；MAVLink 官方资料只作为协议工程参考，不作为制造商要求。

## 来源适用性

- [MAVLink overview](https://mavlink.io/en/about/overview.html)：v1/v2 packet 与协议范围；
- [packet serialization](https://mavlink.io/en/guide/serialization.html)：magic、header、payload
  上限、CRC 覆盖范围、wire CRC 线序和 v2 signature 长度；
- [message signing](https://mavlink.io/en/guide/message_signing.html)：signed flag、13 B signature
  和认证边界；
- [official common.xml](https://raw.githubusercontent.com/mavlink/mavlink/master/message_definitions/v1.0/common.xml)：
  示例 dialect/message definition 来源。示例 `message_id=0, CRC_EXTRA=50` 的 revision 使用
  `master (pin before production)`，只能作为开发 profile；生产必须固定提交/版本并在 profile 中更新。

以上公开资料是协议级一手来源，不是具体飞控、MCU 或设备厂商的版本化制造商要求；本轮不宣称
MISRA、ISO 26262、ASIL、认证或任何嵌入式合规性。

## 实现与简化评估

- `src/serialforge/domain/mavlink.py` 保持纯 stdlib、immutable `MavlinkFrame` 和 280 B packet
  上限；v1/v2 结构、长度、sysid/compid、v2 incompat flags、wire CRC 和签名存在性均在单一
  已分帧 validator 内处理；
- `src/serialforge/domain/codecs.py` 通过显式 `MavlinkCodecConfig`/`MavlinkComponentCodec`
  接入既有 router；mapping 最多 512 项、字段最多 16 项，字段值有界；
- `CRC_EXTRA` 缺失 → `UNVERIFIED`，signed v2 即使 wire CRC 正确也 → `UNVERIFIED`；坏 CRC →
  `INVALID_CHECKSUM`；所有非 `VALID` 字段带 error，Dataset 不会消费；
- `src/serialforge/presentation/viewmodels.py` 和 `main_window.py` 将 MAVLink codec 限制为
  UART 已分帧 RX；TCP/UDP/BLE/RTT 不因本轮扩张；`FrameStatus.UNVERIFIED` 在协议/组件 UI 显示为
  “未验证”；
- 没有新增 transport、线程、动态导入、协议 preset、MAVLink runtime/dialect generator、crypto
  runtime、TX、stream resync、完整 message field generator 或 RTT/J-Link 依赖；这是本轮的行为保持
  简化结果。

## 验证证据

### Domain/profile 手工向量

未创建测试文件，使用临时内存/inline bytes：

- MAVLink CRC-16/MCRF4XX：`crc16_mcrf4xx(b"123456789") == 0x6F91`；
- v1 `FE` + 9 B payload + message id 0/CRC_EXTRA 50 → `VALID`；v2 `FD` + 3 B payload → `VALID`；
- v2 signed flag + 13 B signature + 正确 wire CRC → `UNVERIFIED`，signature 保留 13 B；
- 缺少 CRC_EXTRA → `UNVERIFIED` 且 `calculated_crc is None`；错误 CRC → `INVALID_CHECKSUM`；
- bad magic、sysid/compid 0、未知 incompat flag、截断、超长和多余字节均有对应可见状态；
- profile JSON loads/dumps round-trip、router 14 个字段、坏/未验证字段 error 和 Dataset 跳过均通过；
- 通过 U16 little-endian length boundary shim 送入两个完整 packet，协议 decoder 与 component router
  均得到两个独立结果；该 shim 只证明显式边界复用，不证明 MAVLink stream resync 或 RTU timing。

### Qt / 静态 / 发行

- Qt `offscreen` 实际创建 application/window：UART 载入 `profiles/mavlink-common-heartbeat.json`
  成功；TCP Client、UDP 载入被拒绝；窗口正常关闭；PySide6 仅有无字体目录的既有警告；
- `uv lock --check`：pass；`.\scripts\check.ps1`：pass；Ruff format/check、compileall：pass；
- onedir `.\scripts\package.ps1 -Mode onedir`：pass；EXE WM_CLOSE/退出：pass；
- onefile `.\scripts\package.ps1 -Mode onefile`：pass；bootstrap + GUI WM_CLOSE/退出：pass；
- PyInstaller recursive archive：onedir/onefile 均包含 `serialforge.domain.codecs` 和
  `serialforge.domain.mavlink`；默认 onedir 文件扫描 `Bleak|WinRT|Bluetooth|SEGGER|JLink|probe-rs`
  命中 0；warning 文件 43 行，为平台/可选导入提示；最终无残留 SerialForge 进程；
- 最终包证据：`dist/SerialForge/SerialForge.exe` 3,052,209 B，SHA256
  `CE7DCD2C4B39E64328CADBB2CA51814A45BF98CC67780065540645A7DB69E1F4`；`dist/SerialForge.exe`
  47,547,159 B，SHA256 `733CF71E80D9D446431FB7424AA35B8E32D2E2DDC24A3795023E7CE6A03760E4`。

## 未运行与后续

未连接真实 UART/飞控、未读取完整 common/厂商 dialect mapping、未做 stream resynchronisation、
signature key authentication、TX/ACK/request-response 或飞控 message semantics；未安装/操作
J-Link 驱动、探针或目标板。MAVLink 生产 profile 必须固定官方 definition revision 后再把 wire CRC
报告为 `VALID`；J-Link RTT 按用户要求继续作为最后能力。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
