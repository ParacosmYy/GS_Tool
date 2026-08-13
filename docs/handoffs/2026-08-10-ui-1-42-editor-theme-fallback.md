# 2026-08-10 UI-1.42 批量编辑器主题回退交接

## 目标

消除批量命令编辑器中残留的系统白色 palette：表格左侧冗余 vertical header 白列，以及延时 SpinBox 上下箭头子控件白色区域；
保留用户可配置的步骤、延时、键盘和原生 stepper 行为。

## 实现

- `src/serialforge/presentation/command_batch_editor.py`
  - 隐藏 `QTableWidget.verticalHeader()`；横向“步骤”列继续提供唯一可见序号；
  - 保留 `currentCellChanged`、row selection、表格 AccessibleName/Description 和保存/校验逻辑。
- `src/serialforge/presentation/theme_stylesheet_controls.py`
  - 默认主题补齐 `QSpinBox/QDoubleSpinBox` up/down button、arrow 的背景、hover、disabled 和语义三角箭头；
- `src/serialforge/presentation/theme_variant_controls.py`
  - 三套可选主题使用 `ThemeSpec` 的 surface/hover/disabled/text token 覆盖同一组子控件；
  - 规则只匹配 SpinBox，不影响 QComboBox。

## 架构审查与质量复核

- 架构师 `019fea52-20d7-7283-8bb0-dab5e8f2f30d`（vertical header）和
  `019fea54-8200-7fc3-b095-a29e03980bda`（SpinBox QSS）均等待 30 秒 + 60 秒无返回，已关闭；父代理完成
  Qt header/QSS、键盘/accessibility、主题 token、HIDPI 和行数边界审计，未把超时当作代理结论。
- 独立质量复核 `019fea57-6b07-7d72-b14d-62d09b6ec056` 等待 30 秒 + 60 秒无报告，已关闭；父代理完成
  correctness/Qt-QSS/accessibility/theme/HIDPI/maintainability 五轴复核，未发现必须修正项。
- 简化结论：隐藏重复 header 比为冗余 header 维护主题规则更小；保留原生 SpinBox 只补齐子控件样式，不删除用户调节能力，
  不新增业务状态、timer、资源或跨层依赖。

## 非破坏性验证

```text
scripts/check.ps1       PASS 126 files <= 1000; theme token audit; ruff
UI142_EDITOR            PASS star_trail/moonlit_ocean/sakura_night near_white=0
UI142_HEADER            PASS vertical_header_visible=False; horizontal_header_visible=True; columns=4
UI142_GEOMETRY          PASS table=738x259 at 760x620 dialog
IMPORT/COMPILE          PASS import smoke=125 modules/no QApplication; compileall=pass
```

offscreen 环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。未创建、修改或运行 unit test、mock、fixture、
harness 或 test-only 资产；未运行持续 GUI、EXE 启动、HIDPI/读屏、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件或签名发布验收。

## 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.42`
- size：`47,804,190` bytes
- canonical/root SHA-256：`9B00BD6BB307B4D16C7665ADB1557ED347917B8DE50F0AADB359D443A783F452`
- archive listing SHA-256：`1DE0907DDB8F3A9FA3C0040B991ED18A316A2CD69BDE808402699F8B601DF6B5`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## Embedded R&D assurance gate

本轮仅修改 Python/PySide6 presentation、QSS 和架构文档，不涉及 embedded C/C++、MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、driver、
bootloader、OTA firmware、Flash/NVM、power 或 motor-control；`mcu`、`embedded-enterprise-workflow` 和
`embedded-code-review-simplifier` 的厂商源适用性均为 N/A。未声称 MISRA、ISO 26262、ASIL、ASPICE、WCAG、认证、签名发布或
硬件合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
