# UI-1.44 交接：主题化协议确认弹窗

日期：2026-08-10  
父代理：Codex；共享 checkout 唯一写入者  
范围：presentation-only；未接触 domain/application/infrastructure/transport/hardware

## 结果

协议配置确认弹窗现在有稳定的主题角色和用户可理解的操作语义：确认按钮显示“确认重置/确认应用”（由 action 决定），
取消按钮显示“取消”，默认焦点仍是取消。辅助技术可以读取弹窗、确认和取消按钮的名称与描述，说明会清空哪些派生缓存、
哪些 Raw/记录内容不会被清空。

## 变更文件

- `src/serialforge/presentation/dialog_surface.py`
  - 新增 `configure_confirmation_dialog()`。
  - 只配置 `surfaceRole="confirmation"`、按钮文本/AccessibleName/AccessibleDescription 和已有 `dangerButton` 语义。
  - 复用 `refresh_dynamic_property()`，不创建计时器、不保存状态、不调用业务服务。
- `src/serialforge/presentation/controllers/protocol_config.py`
  - 保留确认条件、协议影响范围文案和标准 `QMessageBox.exec()` 返回判断。
  - 注入 presentation helper，不改变协议解析、清理或传输行为。
- `src/serialforge/presentation/theme_stylesheet_controls.py`
  - 默认主题覆盖 warning surface、label、informative label 和按钮最小宽度。
- `src/serialforge/presentation/theme_variant_controls.py`
  - 三主题 override 使用 warning/surface/text 语义 token。
- `scripts/check_theme_tokens.py`
  - 将 `QMessageBox[surfaceRole="confirmation"]` 纳入必需 selector 集合。
- 文档：ADR 0031、`docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`tasks/plan.md`、`tasks/todo.md`、`docs/handoffs/current.md`。

## 架构审查

架构师角色调用记录：

- 首次 UI-1.44 边界评审：`019fea7a-34a5-7fa1-866f-63b1f8c1a948`，等待两次超时后关闭；父级确认 controller/helper/QSS 三层边界。
- token 导入复核：`019fea7e-9fbc-7891-9244-d0b5c905719f`，等待两次超时后关闭；父级确认仍为同一 presentation 边界。
- Ruff 修复前复核：`019fea80-6376-71d3-b5d4-8d141eb109e5`，等待超时后关闭；父级确认移除冗余 `f` 前缀不改变行为。

独立只读复核结论：helper 是稳定的单一职责复用边界；保留现有 `dangerButton`、`refresh_dynamic_property` 和主题控制模板，
没有为减少行数而改变 QMessageBox 生命周期、按钮 role、默认焦点、协议清理条件或错误语义。后续风险单独记录：重置文案与
editor 行为的覆盖范围、仅有未完成帧/仅统计状态的确认门，以及真实 Windows Qt style 的 objectName/pixel 差异。

## 验证证据

```text
scripts/check.ps1       PASS source line limit: 127 files <= 1000
                        PASS theme token audit: 3 themes, 22 semantic tokens, 13 selectors, legacy_qss_literals=320
                        PASS Ruff
UI144_IMPORT            PASS 126 modules; QApplication.instance() is None after import
UI144_COMPILE           PASS python -m compileall -q src
UI144_CONFIRM           PASS star_trail/moonlit_ocean/sakura_night;
                        surfaceRole=confirmation; default_cancel=True; confirm=dangerButton;
                        standard button text; AccessibleName/Description; near_white=0
```

离屏环境警告：`QFontDatabase: Cannot find font directory .../PySide6/lib/fonts`；这不代表用户 Windows 环境字体缺失。
已保存视觉复核：

- `build/ui_review_ui144_confirmation_star_trail.png`
- `build/ui_review_ui144_confirmation_moonlit_ocean.png`
- `build/ui_review_ui144_confirmation_sakura_night.png`

没有创建、修改或运行测试专用资产；未启动持续 GUI/EXE，未运行网络、设备、硬件、OTA、RTT/J-Link 或正式签名发布验收。

## 嵌入式 assurance gate

适用性为 N/A：项目是 Python/PySide6 Windows 桌面应用，源码无 MCU、C/C++、固件、BSP/HAL/CMSIS、RTOS、ISR/DMA 或
硬件目标；无适用的一手厂商文档，也未声称 MISRA、ISO 26262、ASIL、ASPICE 或认证合规。独立复核、简化评估和非破坏性
验证已记录；固件 build/map/HIL/硬件验证不适用，Windows 原生 GUI/读屏/HIDPI 仍待授权验收。

## 最终打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，路径已校验在当前 workspace 内
- source revision：`local-ui-1.44`
- size：`47,804,976` bytes
- canonical/root SHA-256：`6D58AF332F7BA492973C0075DF1ECE12086A4A6892C7FE95862B29CDE304AC38`
- archive listing SHA-256：`83B52335762EE094ACE2816DC018D5E7D4AFCB48A6555893C0A0D8FAC0FF3331`
- provenance：`PROVENANCE.json` verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`
- 未启动 EXE；只验证 canonical/root size 与 SHA-256 一致。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
