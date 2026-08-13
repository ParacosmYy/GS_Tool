# UI-1.99 协议/遥测页选项与动态文案交接

日期：2026-08-11  
范围：协议解析、Component、Dataset、Curve、历史回放 presentation surface

## 结果

- 协议 preset、帧格式、校验、最大帧、分隔符 Hex、长度前缀、字节序和回放速度改为更清晰的中文 bounded selector/辅助说明；下拉均不可手动输入。
- `ProtocolPreset`、`FramingKind`、`ChecksumKind`、长度字节 int、字节序 string、回放速度 float 等底层 `itemData` 保持不变；应用、重置、解析 gate、专用帧动态启停不变。
- Component/Dataset/Curve 的 status、空态、流水线摘要、曲线可访问描述和 Dataset 动态刷新路径统一使用“序列/样本/原始终端”等用户文案，消除了动态刷新后重新出现的 `series/sample/Raw` 英文 fallback。
- 视觉仍复用既有三主题 semantic token、DatasetCurveWidget 的共享动画/lifecycle timer 和现有 snapshot；未新增业务时钟、协议能力、设备 I/O 或依赖。

## 角色与独立复核

```text
产品角色       019fec7e-0688-76a3-8fe0-3408724f14f2  called; wait timed out; closed
架构角色       019fec7e-06d0-74f0-a713-f9408676035c  called; wait timed out; closed
UI 设计角色    019fec7e-071f-72b3-878f-272d5e19b55c  called; wait timed out; closed
开发角色       019fec7e-076c-7872-8a60-bccb988c5c2c  called; wait timed out; closed
验证角色       019fec7e-07b8-7a50-92f4-8baf61525d61  called; wait timed out; closed
打包角色       019fec7e-0803-7ad1-851d-1c163441998f  called; wait timed out; closed
产品角色（动态刷新复核） 019fec81-4914-7040-abf0-470d9cb65183  called; wait timed out; closed
架构角色（动态刷新复核） 019fec81-4964-7e01-9ee6-c0a448f22bc5  called; wait timed out; closed
UI 设计角色（动态刷新复核） 019fec81-49ac-7d53-b8c2-8078474f9761  called; wait timed out; closed
开发角色（动态刷新复核） 019fec81-4a00-77e0-b93b-9e3d7e969e8b  called; wait timed out; closed
验证角色（动态刷新复核） 019fec81-4a4a-74e2-b9ab-5d505f16a881  called; wait timed out; closed
打包角色（动态刷新复核） 019fec81-4a96-72a1-8b0a-17ea53c3b561  called; wait timed out; closed
独立质量复核 / embedded simplifier 019fec84-778b-72f3-a66f-5844c72a4cf7  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成五轴审查：correctness 确认 label/data、信号和动态 gate；readability/simplicity
确认复用既有 bounded combo、currentText projection 与 DatasetCurveWidget owner，没有新增通用框架；architecture 确认 presentation 只消费既有 DTO/enum；
security 确认无新增输入、I/O、依赖或敏感数据；performance 确认没有新增 timer、线程或热路径工作。简化评估结论为“复用既有 owner/helper，未做行为性删减”。

Embedded R&D 记录：本轮没有嵌入式 C/C++/MCU/SDK/RTOS/bootloader/Flash/真实 OTA/debug 修改；vendor public source applicability=N/A，未声明任何厂商要求或认证合规。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI199_PROTOCOL_OPTIONS_VECTOR_PASS themes=3 preset_labels=8 framing_labels=6 checksum_labels=5 typed_data=1 dynamic_gates=1 dataset_refresh=1 replay_affordance=1
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

真实组合根通过 Qt offscreen 内存向量，未显示主窗口；仅出现 PySide6 环境缺少 fonts 目录的 Qt warning，不影响断言。
未运行真实串口、网络、BLE、协议设备、HIDPI、读屏、硬件、EXE 启动、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.99
size: 47,925,745 bytes
SHA-256: 5DA3C31E0009294AAC921427A94F01BCA578C6A0AD58E2971C1D8193D319F0C4
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
