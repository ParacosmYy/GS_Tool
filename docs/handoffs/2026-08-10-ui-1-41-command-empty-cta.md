# 2026-08-10 UI-1.41 批量命令空态 CTA 交接

## 目标

让命令管理页在首次进入时直接给出下一步操作，并确保空态卡片在有限 workspace viewport 内完整可见；不改变批量
命令的 domain/application 行为、执行 gate、snapshot 或 transport 语义。

## 实现

- `src/serialforge/presentation/action_surface.py`
  - 新增通用 `ActionRailButton(QPushButton)`；保留原生 Qt paint/click/focus/accessibility/QSS 契约；
  - 只消费共享 `(phase, animated)` 与 `ThemeSpec.accent`，绘制底部 rail；没有 timer、业务状态或 ViewModel 依赖。
- `src/serialforge/presentation/command_batch_empty_state.py`
  - 新增 `CommandBatchEmptyState` 和无资源 `CommandBatchGlyph`；
  - 公开 `new_requested` 无参数 intent、`set_frame()`、`stop()` 和 gate 可控的 `action_button`；
  - CTA 文案、hint、tooltip、AccessibleDescription 都在 presentation surface 内维护。
- `src/serialforge/presentation/terminal_surface.py`
  - `TerminalActionButton` 保留兼容导出，指向 `ActionRailButton`，移除重复 painter 实现。
- `controllers/terminal.py`
  - 组装空态并把 `new_requested` 显式接到 `commands.py` 的既有 `new_command_batch_action`。
- `controllers/commands.py`
  - `batch is None` 时隐藏 `CommandBatchSurfaceLabel` 与空结果 `QTableWidget`，显示完整空态；
  - 有 batch 时恢复 status surface/结果表，空态隐藏；不修改 snapshot、selection、执行顺序或 connection refresh 语义。
- `controllers/connection.py` / `controllers/composition.py` / `controllers/lifecycle.py`
  - 空态 CTA 服从同一 batch-edit lock、键盘顺序和共享 frame/stop fan-out。
- `theme_stylesheet_controls.py` / `theme_variant_controls.py`
  - 为新空态卡片、标题、hint 增加默认和 tokenized 三主题样式，未引入白色系统 palette 回退。

## 架构审查与质量复核

- 架构师调用：`019fea3f-85ac-7f12-bc5c-5ccbb81d5b96`、`019fea46-8669-7943-9c6c-74a4a908e5a4`、
  `019fea48-ea6c-7d73-afcc-a3fe37e77e67`；每次均等待 30 秒 + 60 秒无返回，已关闭；没有把超时当作代理结论，父代理
  完成模块边界、Qt signal/lifetime、gate、响应式和共享时钟审计后继续。
- 独立质量复核：`019fea4b-2da9-7d21-9b5c-a73117959a8a`；等待 30 秒 + 60 秒无返回，已关闭；父代理完成
  correctness/architecture/accessibility/performance/maintainability 五轴复核，未发现必须修正项。
- 简化结论：把可复用 rail 从 terminal surface 抽离，保留 single owner/action path；空态只表达 intent；无 batch 时
  隐藏不具信息价值的空 status/table，减少 viewport 占用，不复制任何业务状态。

## 非破坏性验证

```text
scripts/check.ps1       PASS 126 files <= 1000; theme token audit; ruff
IMPORT_SMOKE            PASS modules=125 qapplication_instance=False
UI141_EMPTY             PASS component signal; shared frame; CTA visible
UI141_VISIBILITY        PASS empty status/table hidden; batch status/table restored; result row=1
UI141_RESPONSIVE        PASS 980: 910x98, 1180: 1110x98; CTA width=106; no clipping
UI141_THEME             PASS star_trail/moonlit_ocean/sakura_night near_white=0
UI141_LIFECYCLE         PASS pause static; hide static; close timer=False
```

offscreen 仅用于离屏 Qt 组装、状态投影和截图；环境提示缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。
没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。未运行持续 GUI、EXE 启动、HIDPI/读屏、真实
传输、OTA/RTT/J-Link、硬件或签名发布验收。

## 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.41`
- size：`47,803,326` bytes
- canonical/root SHA-256：`D21835D2C436FA2245AF58CF4063D4B6C03A2562E83A726FF82CCF12ACCC107A`
- archive listing SHA-256：`1DE0907DDB8F3A9FA3C0040B991ED18A316A2CD69BDE808402699F8B601DF6B5`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## Embedded R&D assurance gate

本轮仅修改 Python/PySide6 presentation、主题样式和架构文档，不涉及 embedded C/C++、MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、
driver、bootloader、OTA firmware、Flash/NVM、power 或 motor-control；`mcu`、`embedded-enterprise-workflow` 和
`embedded-code-review-simplifier` 的厂商源适用性均为 N/A。未声称 MISRA、ISO 26262、ASIL、ASPICE、WCAG、认证、签名发布或
硬件合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
