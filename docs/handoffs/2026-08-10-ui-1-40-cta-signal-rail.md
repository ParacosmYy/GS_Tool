# 2026-08-10 UI-1.40 终端空态 CTA 共享 frame signal rail 交接

## 目标

在 UI-1.39 空态导航 CTA 基础上补充轻量的二次元 signal rail，让按钮与终端观测网格共享视觉节奏；
不增加新的动画时钟，不改变连接导航和 session/transport 行为。

## 实现边界

- `src/serialforge/presentation/terminal_surface.py`
  - 新增 `TerminalActionButton(QPushButton)`，保留原生按钮文字、点击信号、focus/accessibility 与 `primaryButton` QSS；
  - `paintEvent()` 先完成原生按钮绘制，再用 `ThemeSpec.accent` 和 phase 在底部绘制低对比度移动 rail；
  - `set_frame()/stop()` 只维护 renderer phase/animated，不创建 timer、不读取业务状态。
- `TerminalEmptyState.set_frame/stop` 转发到 action button；现有 `lifecycle._motion_surfaces` 已包含空态，
  `MotionController` 仍是唯一 frame source。paused/history CTA 不可见，因此不显示 rail。
- 新增 [`ADR 0027`](../adr/0027-terminal-action-shared-frame-rail.md)，同步架构树、约束、计划和 todo。

## 架构师调用与质量复核

- 架构师 `019fea2f-34fd-7f91-9122-599a2f641b12`：两个等待窗口（30/60 秒）无可用结论，已关闭；父代理完成
  Qt paint/QSS、parent/lifetime、shared frame、focus/accessibility、响应式和生命周期审计后实施。
- 独立质量审查 `019fea37-90b8-7191-b136-fcd4537b2869`：两个 60 秒等待窗口无报告，已关闭；父代理按
  correctness/readability/architecture/security/performance 五轴审查，未发现必须修正项。
- 行为保持简化：没有新增业务字段、第二个 QTimer、输入解析、依赖或连接副作用；实现保持在 terminal surface 单一变化边界。

## 非破坏性验证

```text
scripts/check.ps1       PASS 124 files <= 1000; theme token audit; ruff
IMPORT_SMOKE            PASS modules=123 qapplication_instance=False
UI140_RAIL              PASS phase=π/2↔3π/2 moving_diff=34; stop animated=False
UI140_SHARED_FRAME      PASS phase 0.000 -> 0.300; animated=True; timer=True
UI140_PAUSE             PASS timer=False; animated=False
UI140_RESPONSIVE        PASS 1180x780 surface=1152x192 card=378x128 bottom=159; hscroll=False; maximum=0
UI140_NAV               PASS actual CTA click Tab 1 -> Tab 0; focus restored; session state unchanged
UI140_THEME             PASS star_trail/moonlit_ocean/sakura_night near_white=0
UI140_VISIBILITY        PASS hide static; close timer=False
```

截图：

- `build/ui_review_ui140_cta_rail_star_trail.png`
- `build/ui_review_ui140_cta_rail_moonlit_ocean.png`
- `build/ui_review_ui140_cta_rail_sakura_night.png`
- `build/ui_review_ui140_cta_rail_1180.png`

第一次像素验证使用 `0` 与 `π`，两者在 `sin()` 下同为中心位置，构成验证脚本 false negative；已按调试流程定位，
改用 `π/2` 与 `3π/2` 后 moving_diff=34 通过。offscreen 环境缺少 PySide6 fonts directory，中文方框仅为环境告警。
没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产；未运行持续 GUI、EXE 启动、HIDPI/读屏、
真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件或签名发布验收。

## 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.40`
- size：`47,793,505` bytes
- canonical/root SHA-256：`B81A72954F75835E11CE38832AC1F5CEE4768CD6310D4A306352ABE1F8D67A6B`
- archive listing SHA-256：`0A1F0D57EB1DA293E6ABFD1384653D9CAB802DEC5AF7FC91091702B4F618C26E`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## Embedded R&D assurance gate

本轮仅修改 Python/PySide6 presentation、架构文档和 handoff，不涉及 embedded C/C++、MCU、BSP/HAL/CMSIS、RTOS、
ISR/DMA、driver、bootloader、OTA firmware、Flash/NVM、power 或 motor-control；`mcu`、
`embedded-enterprise-workflow` 和 `embedded-code-review-simplifier` 的厂商源适用性均为 N/A。未声称 MISRA、
ISO 26262、WCAG、认证、签名发布或硬件合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
