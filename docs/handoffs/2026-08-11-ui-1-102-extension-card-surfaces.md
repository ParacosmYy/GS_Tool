# UI-1.102 扩展工具站能力卡与接入概览主题表面交接

日期：2026-08-11  
范围：嵌入式扩展工具站 presentation surface

## 结果

- `QFrame#extensionStationOverview` 增加 info surface、accent 左色带和状态牌；metric value 增加可读层级。
- `QFrame#extensionCapabilityCard` 增加基础 themed card，并按 `contract_only` / `attach_only` 投影 info/blue 与 history/purple 语义色带。
- base stylesheet 与 variant shell 对称；不改变 7 张卡片、DTO、只读/accessibility 文案、动作、生命周期或 OTA/debug contract。

## 角色与独立复核

```text
产品角色       019fec98-e2a0-71f0-9cd6-bcfef160c94d  called; wait timed out; closed
架构角色       019fec98-e2eb-7892-97e5-f10ae3e69c04  called; wait timed out; closed
UI 设计角色    019fec98-e336-7523-9080-9885b562b9c6  called; wait timed out; closed
开发角色       019fec98-e382-78e0-a0ab-76ef24853141  called; wait timed out; closed
验证角色       019fec98-e3d1-7a92-bd39-a93685c7bba9  called; wait timed out; closed
打包角色       019fec98-e41e-7dd2-979e-b79efb9d8cf7  called; wait timed out; closed
修正前置六角色 019fec9a-44fd-76b1-a97f-cd476ee2444f / 019fec9a-4550-7083-88a2-890fd5630136 / 019fec9a-459c-7c43-85ec-1784857468c9 / 019fec9a-45eb-7fc1-9215-79ea6e580f8b / 019fec9a-463c-71d0-88e7-7c4701540907 / 019fec9a-468a-7980-80d3-12f5f5049b65  called; wait timed out; closed
独立质量复核 / embedded simplifier 019fec9a-f3f2-7040-b9d1-7c6f69dfb102  called after implementation; wait timed out; closed
```

所有角色与独立复核均未返回完整报告，超时不视为通过。父代理完成 correctness、readability、architecture、security、performance 五轴复核；首轮
`ThemeSpec.border_strong` 字段错误已定位并在第二轮角色复核后改为已有 `theme.border`，没有新增 token 或抽象。简化评估为复用现有 QSS
semantic token 和 dynamic property。

嵌入式 R&D 适用性：N/A。未修改嵌入式 C/C++、MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、bootloader、Flash、真实 OTA 或硬件 debug；未声明
厂商要求、MISRA、ISO 26262、认证或硬件合规。

## 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src        PASS
.venv\Scripts\python.exe -m ruff check src           PASS
UI102_EXTENSION_SURFACE_VECTOR_PASS themes=3 cards=7 contract_only=5 attach_only=2 overview=1 renders=24 near_white_pixels=0
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

真实组合根使用 Qt offscreen，未显示主窗口；仅有 PySide6 fonts 目录 warning。未运行可见 GUI/HIDPI/读屏、真实设备/网络、EXE 启动、OTA、签名、
硬件或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.102
size: 47,928,081 bytes
SHA-256: 717F20E69CFA0120A1DB833509FEEBCDB37AA66199559A6273B75FEF3E1DD9F1
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

`SerialForge.exe` 是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
