# M5g 六角色复核记录：NMEA 0183 RX line checksum profile

日期：2026-08-09  
范围：NMEA 0183 Line framing、`$...*HH` XOR 校验、格式/校验错误可见、预设/UI、静态门和双模式打包。  
源码写入者：父代理；六角色子代理均为只读复核，无子代理直接修改当前 checkout。

## 六角色记录

| 角色 | 子代理 run | 结论 | 关键建议 |
|---|---|---|---|
| 产品 | `019fe4ad-bb2e-7502-a6db-1080d32b04d8` | pass with conditions → 已整合 | 只交付 RX checksum profile；不宣称完整 NMEA 版本、字段或认证兼容 |
| 架构 | `019fe4ad-bb69-7082-ba78-d7e0621d7dc6` | pass with conditions → 已整合 | 复用 `ProtocolConfig.LINE` 和 bounded decoder；保留 raw truth，不让 component router 负责 framing |
| UI 设计 | `019fe4ad-bba7-7a82-a3d0-e30a7eee1391` | pass → 已整合 | 预设/校验选项保持现有协议面板；非 UART/TCP Client 继续 raw-only |
| 开发 | `019fe4ad-bbe4-78f0-bbf6-98c3dd3a5947` | revise → 已整合 | NMEA `*HH` 不能复用普通 XOR8；大小写 hex、`*` 结构、CRLF、格式状态需显式处理 |
| 验证 | `019fe4ad-bc25-7683-bf2b-bbd1ff176133` | pass conditionally → 已整合 | 正/坏 checksum、坏 hex、缺失/重复 `*`、分片、raw、offscreen、包门；真实 GNSS 后置 |
| 打包/流程 | `019fe4ad-bc65-7fe3-ab0c-967544c22748` | revise → 已整合 | 无新增依赖；必须在 M5g 后重建 onedir/onefile 并记录扫描、启动和 hash |

六个子代理已完成并关闭；父代理负责整合、最终源码检查和验证。

## 公共来源适用性

- NMEA 官方：[NMEA 0183](https://www.nmea.org/nmea-0183.html)，用于确认标准归属、版本/版权边界；
  官方页面说明标准正文受版权保护，因此仓库不复制完整标准文本、句型表或认证结论；
- Quectel 第一方公开文档：[LC29H(BS) GNSS Protocol Specification V1.0](https://www.quectel.com/content/uploads/2024/02/Quectel_LC29HBS_GNSS_Protocol_Specification_V1.0.pdf)，
  §2.1、适用 LC29H(BS) 设备，用于 `$` 起始、`*` 分隔、两个 ASCII hex 和 `$`/`*` 之间 XOR
  的工程向量参考；它不是所有 NMEA 设备的制造商要求，实际设备需按授权手册确认。

当前 checkout 是 Python/PySide6 桌面应用，没有 C/C++、MCU、BSP/HAL、RTOS、固件或硬件配置改动；
因此嵌入式厂商要求适用性为 N/A，未作 MISRA、ISO 26262、认证或 NMEA 合规声明。没有烧录、部署、
连接 GNSS/UART 或操作目标硬件。

## 变更摘要

- `ChecksumKind.NMEA0183` 只允许 `FramingKind.LINE`；复用既有 LF/CRLF bounded line parser；
- 新增 `FrameStatus.INVALID_FORMAT`，NMEA sentence 结构/hex 错误与 checksum 数值不匹配分开显示；
- NMEA 要求 `$` 开始、唯一 `*`、非空 ASCII body、尾部正好两个 ASCII hex；大小写 hex 均可；
- valid 与 checksum-invalid 派生 payload 去除 `*HH`，raw ingress、terminal 和 JSONL recorder 不变；
- `ProtocolPreset` 增加 NMEA 0183 Line + XOR，协议面板增加 NMEA 选项；选择不改变 parser，点击“应用”才生效；
- 不解析 RMC/GGA/AIS 字段、不做 TX 编码/自动补 checksum/ACK/重试、不覆盖 NMEA 版本/方言全集、
  不处理 RTCM 混流、不引入第三方 NMEA 库或动态代码。

## 未决风险

`ProtocolFramesDecodedEvent` 仍没有 parser generation；配置切换时已完成解析但尚未发布的旧事件可能
迟到，这是后续 source/epoch gating 切片，不在 M5g 扩大事件 schema。`DecodedFrame` 也没有单独
`wire_payload` 字段，但现有 raw recorder/terminal 保留完整线缆字节；派生 payload 的后缀去除不覆盖事实来源。

真实设备还可能有厂商方言、不同 NMEA 版本、混流数据、噪声/丢字节、非 ASCII 扩展或 proprietary sentence；
这些不由当前 profile 静默接受或宣称兼容。

## 验证证据

已运行：

- `uv lock --check`：通过；
- `uv run --locked ruff format --check --no-cache src`：通过，48 files；
- `uv run --locked ruff check --no-cache src`：通过；
- `uv run --locked python -m compileall -q src`：通过；
- `scripts/check.ps1`：通过；
- inline NMEA vectors：公开设备规格示例、正确 checksum、坏 checksum、坏 hex、缺失/重复 `*`、非 `$` 起始、
  CRLF、跨读取分片、大小写 hex、禁止非 Line framing、`INVALID_FORMAT`/`INVALID_CHECKSUM` 区分和 `*HH` 去除：通过；
- Qt `QT_QPA_PLATFORM=offscreen`：NMEA preset 选择不提前应用、显式应用、UDP raw-only 禁用和窗口/worker 关闭：通过；
  已知 `QFontDatabase` 缺少 PySide6 字体目录提示，不影响退出；
- `scripts/package.ps1 -Mode onedir`：通过；实际 GUI startup/WM_CLOSE/exit，PID `82556`；
- `scripts/package.ps1 -Mode onefile`：通过；实际 bootstrap/GUI child startup/WM_CLOSE/exit，bootstrap `92720`、GUI `18988`；
- 默认 onedir 文件名扫描中的 `Bleak|WinRT|Bluetooth|SEGGER|JLink|probe-rs` 匹配数：`0`；
- PyInstaller analysis archive 已包含 `serialforge.domain.protocol_presets`；
- 验证结束时没有残留由本轮启动的 `SerialForge` EXE 进程。

最终产物：

| 产物 | 大小 | SHA-256 |
|---|---:|---|
| `dist/SerialForge/SerialForge.exe` | 3,031,402 B | `B6A12B47470E46F2FFD0AA4AE008832CB37BFE60067D9223B574CC4BBB3052EA` |
| `dist/SerialForge.exe` | 47,525,678 B | `B50256A70F8572E461711C38B97289BF85330BBCE083C38811D349149F00A2CE` |

## 简化评估

复用现有 `ProtocolConfig`、Line parser、`DecodedFrame`、raw recorder、component event 和 UI 面板，
只增加一个显式 checksum 分支、一个状态值和一个 preset；没有新增 transport/application port、线程、
依赖或第三方协议库。结构错误与数值错误分开后，错误语义更清晰；完整字段 codec 和 generation gating
保留为后续独立切片，避免把标准协议例外扩散到所有传输。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
