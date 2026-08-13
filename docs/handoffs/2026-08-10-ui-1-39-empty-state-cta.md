# 2026-08-10 UI-1.39 终端空态连接导航 CTA 交接

## 目标

让无 RX 数据的终端空态提供可发现的下一步动作，同时保持 presentation 与连接/传输业务高内聚、低耦合；
用户点击后回到链路连接 Tab，但不会发生自动连接或 session 状态变化。

## 实现

- `src/serialforge/presentation/terminal_surface.py`
  - 增加无参数 `connection_requested` signal；CTA 复用 `primaryButton` 主题选择器；
  - idle/waiting/transition 可见，paused/history 隐藏；提供 accessible name/description/tooltip；
  - 调整空态卡片内边距以保证 980×680 下 CTA 不溢出；不新增 timer、token、业务状态或外部资源。
- `src/serialforge/presentation/controllers/bootstrap.py`
  - 用显式 `partial` 把 CTA intent 接入 workspace navigation owner。
- `src/serialforge/presentation/controllers/workspace_runtime.py`
  - 新增 `select_workspace_tab(window, index)`，仅处理 closing、Tab count/index、真实 Tab 切换和焦点恢复；
  - 不调用连接动作、不读取 transport、不修改 ViewModel/session。
- 架构与约束：新增 ADR 0026；同步 `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`tasks/plan.md`、`tasks/todo.md`。

## 架构师调用与质量复核

- 架构师 `019fea22-d380-7181-a745-0c70ae618a09`：两个 30/60 秒等待窗口无可用结论，已关闭；父代理完成
  presentation signal、Qt parent/lifetime、导航 owner、焦点/无障碍、响应式与无副作用审计后才继续。
- 独立质量审查 `019fea28-6ca3-72c1-938c-ca5dd09f1ff4`：两个 60 秒等待窗口无可用报告，已关闭；父代理按
  correctness/readability/architecture/security/performance 五轴复核，未引入新依赖、输入解析或不受控副作用。
- 简化评估：保留一个空态组件、一个无参数 intent、一个 workspace navigation owner；复用既有 QSS/shared motion，
  没有复制 Tab 状态、增加第二时钟或把连接逻辑塞进 UI。

## 非破坏性验证

```text
scripts/check.ps1       PASS 124 files <= 1000; theme token audit; ruff
IMPORT_SMOKE            PASS modules=123 qapplication_instance=False
EMPTY_980               PASS card=378x128 inside terminal=952x170; actual click from Tab 2 -> Tab 0; focus restored; session unchanged
EMPTY_STATES            PASS waiting=True transition=True paused=False history=False idle=True
EMPTY_1180              PASS surface=1152x192; card=378x128; hscroll_visible=False; maximum=0; near_white=0
THEMES                  PASS star_trail/moonlit_ocean/sakura_night near_white=0
```

截图：

- `build/ui_review_ui139_empty_cta_star_trail.png`
- `build/ui_review_ui139_empty_cta_moonlit_ocean.png`
- `build/ui_review_ui139_empty_cta_sakura_night.png`
- `build/ui_review_ui139_empty_cta_1180.png`

offscreen 环境缺少 PySide6 fonts directory，中文方框仅是环境告警；未进行持续 GUI、HIDPI/读屏、EXE 启动、
真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件或签名发布验收。没有创建、修改或运行 unit test、mock、fixture、
harness 或 test-only 资产。

## 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.39`
- size：`47,796,629` bytes
- canonical/root SHA-256：`4FE6A2C69C12EDCBD5639FB163A332B5E38D62CFE4803C7F9B731F34AED63988`
- archive listing SHA-256：`0A1F0D57EB1DA293E6ABFD1384653D9CAB802DEC5AF7FC91091702B4F618C26E`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## Embedded R&D assurance gate

本轮仅修改 Python/PySide6 presentation、架构文档和 handoff，不涉及 embedded C/C++、MCU、BSP/HAL/CMSIS、RTOS、
ISR/DMA、driver、bootloader、OTA firmware、Flash/NVM、power 或 motor-control；`mcu`、
`embedded-enterprise-workflow` 和 `embedded-code-review-simplifier` 的厂商源适用性均为 N/A。未声称 MISRA、
ISO 26262、WCAG、认证、签名发布或硬件合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
