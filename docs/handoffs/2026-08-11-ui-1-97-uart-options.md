# UI-1.97 UART 选项用户化交接

日期：2026-08-11  
范围：UART 下拉选项的中文文案、不可编辑约束与 accessibility 说明

## 结果

- 波特率保留 25 个常用预设并显式不可编辑，继续避免用户手输任意数字。
- 数据位显示 `5 位/6 位/7 位/8 位`；校验显示无/奇/偶/Mark/Space；停止位显示 `1 位/1.5 位/2 位`；流控显示无流控、软件流控和两种硬件流控。
- 所有五个 selector 均补齐 tooltip/accessibility description；itemData、domain enum、快速配置与连接行为保持不变。

## 角色与独立复核

```text
产品角色       019fec6f-4f9a-7322-9ef0-89983bf1a185  called; wait timed out; closed
架构角色       019fec6f-4ffb-7971-9c36-75cb8d6eef32  called; wait timed out; closed
UI 设计角色    019fec6f-5051-78e1-93d6-6a84d9b9823d  called; wait timed out; closed
开发角色       019fec6f-50a4-7ab2-8558-b944892bdf0a  called; wait timed out; closed
验证角色       019fec6f-50f3-7162-8dd5-18b4027ce669  called; wait timed out; closed
打包角色       019fec6f-513e-72f3-80db-a10f64652aae  called; wait timed out; closed
独立质量复核   019fec70-a9d0-7a21-8ef9-729885af926a  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成五轴审查：correctness 确认 label/data 分离与 enum 重建；readability/simplicity 确认仅改变 option 文案和 affordance；architecture 确认不触碰 domain/application/transport；security 确认不增加输入、I/O 或依赖；performance 确认无额外 timer、线程或热路径工作。嵌入式 C/C++ 适用性：N/A。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI197_UART_OPTIONS_VECTOR_PASS themes=3 baud_presets=25 localized=data/parity/stop/flow non_editable=1 typed_data=1
```

真实组合根通过 Qt offscreen 内存向量，未显示主窗口；仅出现 PySide6 环境缺失 fonts 目录的 Qt warning，不影响 selector/value 断言；未运行真实串口、HIDPI、读屏、硬件、签名或正式发行验收。

## 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.97` 生成并覆盖根目录
`SerialForge.exe`；canonical/root 字节一致，仓库 provenance verifier 通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.97
size: 47,920,572 bytes
SHA-256: D4014B1E307BABA9926339CA04FEDCE37F6E7FFD8813D9CBCEEE30AE58AC54BE
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
