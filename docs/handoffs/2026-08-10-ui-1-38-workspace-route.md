# 2026-08-10 UI-1.38 工作区路线 beacon 交接

## 目标与根因

为“链路 / 连接、协议 / 遥测、命令管理”三页增加持续可读的视觉路线锚点，让既有 180ms workspace page fade 不再是孤立的
过渡效果。首次采用 `QTabWidget.setCornerWidget()` 的方案被离屏几何证据否定：980px 下固定 132×28 widget 的 geometry 为
`x=953`、父容器宽 `952`，导致实际渲染被裁切。

## 实现边界

- 新增 `src/serialforge/presentation/workspace_route_surface.py`：`WorkspaceRouteSurface` 只消费 bounded Tab index、ThemeSpec 和
  shared frame；固定 132×28，NoFocus、鼠标透明、空 accessibility，不读 ViewModel/domain、不创建 timer。
- `controllers/workspace.py` 返回 `workspaceShell`，由纵向 layout 承载真实 `QTabWidget` 与右对齐 `workspaceRouteStrip`；真实 Tab
  widget 仍通过 `window._workspace_tabs` 暴露给既有 controller，currentChanged、Tab 顺序、页面 transition 和无障碍文案不变。
- `controllers/workspace_runtime.py` 只把现有 `currentChanged` index 投影到 route；`controllers/lifecycle.py` 将 route 加入已有共享
  motion surfaces，隐藏/最小化/暂停/低动效/关闭沿用统一静态停止。

## 架构师调用、调试与质量复核

- 架构师 Luna max `019fea16-2615-75b0-99fc-8c75b786f434` 对 corner/strip 方案两次等待未返回，已关闭，未把超时当作 GO；父代理按
  Qt ownership、响应式空间、生命周期和可回滚性完成审计。
- 发现裁切后停止扩展功能，复现并定位到 Qt corner geometry；修复为 layout-owned strip。格式修复架构师
  `019fea18-6a20-7a53-8f27-8efad61c45d1` 同样超时关闭，未给出结论。
- 代码质量复核：正确性、可读性、架构、无输入安全风险、性能、NoFocus/accessibility 均通过；新增文件低于 200 行。

## 验证证据

```text
scripts/check.ps1      PASS 124 files <= 1000; theme token audit; ruff
IMPORT_SMOKE           PASS modules=124 qapplication=none
ROUTE_RESPONSIVE       980x680 shell=952 tabs=952x220 route=814,3,132,28 visible=True hscroll=0
ROUTE_RESPONSIVE       1180x780 shell=1152 tabs=1152x220 route=1014,3,132,28 visible=True hscroll=0
UI138_ROUTE            PASS indices=[0,1,2] frames=15 hidden_timer=False restored_timer=True closed_timer=False
UI138_CONTROLS         PASS pause=(False,False) resume=(True,True) reduced=(False,False) restore=(True,True) close=(False,False)
THEME_AUDIT            star_trail/moonlit_ocean/sakura_night near_white=0
SCREENSHOT             build/ui_review_ui138_route_980.png (人工查看通过)
```

offscreen 环境提示 PySide6 fonts directory 缺失，中文方框仅是该环境字体告警；没有运行持续 GUI、读屏、HIDPI、CPU/内存 profile、
EXE 启动、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA 或目标硬件。没有创建或运行测试专用资产。

## 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录副本：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.38`
- size：`47,795,336` bytes
- SHA-256：`456722C6D7D5B0F5F64A23FE0C9EEF61E2508E42CFBD2153E38662BCD6C8F735`
- archive listing SHA-256：`0A1F0D57EB1DA293E6ABFD1384653D9CAB802DEC5AF7FC91091702B4F618C26E`
- signature：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## Embedded R&D assurance gate

本轮仅修改 Python/PySide6 presentation，不涉及 embedded C/C++、MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、driver、bootloader、OTA firmware、
Flash/NVM、power 或 motor-control；`mcu`、`embedded-enterprise-workflow` 和 `embedded-code-review-simplifier` 的厂商源适用性均为 N/A。
未声称 MISRA、ISO 26262、WCAG、认证、签名发布或硬件合规。

简化评估：保留单一 MotionController 和现有 lifecycle fan-out；删除错误的 corner slot 依赖，使用一个 focused renderer 与 layout-owned
strip，未复制业务状态、未新增 timer、未改变 Tab keyboard/accessibility contract。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
