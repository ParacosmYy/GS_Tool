# ARCH-123 / UI-1.196 命令批处理空态与页面节奏交接

日期：2026-08-12  
范围：Python/PySide6 presentation-only；无 embedded C/C++、固件、硬件或厂商 SDK 改动，public-vendor-source applicability=N/A。

## 变更

- `command_batch_empty_state.py`：空态改为自然高度；新增 `CommandBatchStepCard` 与 `CommandBatchStepRail`，三步引导只表达新建、添加步骤、
  执行查看的静态路径。初始化和 resize 共用幂等 `_relayout()`，不会依赖首个 resize 才消除同格卡片。
- `controllers/command_workspace_builder.py`：空态不再使用 root flexible stretch；保留既有 `new_requested` 与批处理动作绑定。
- `responsive_scroll_area.py`、`controllers/composition.py`、`controllers/workspace.py`：增加 `ShortPageVerticalRhythm`；命令页使用
  `TOP`，其他页面默认 `CENTER`。滚动 owner、viewport、业务和页面 API 保持兼容。
- `theme_stylesheet_controls.py`、`theme_variant_controls.py`：新增三步卡语义 selector，复用既有 theme token，无白色硬编码。

## 评审记录

- 架构师主裁决 `019ff58a-9d3d-75b0-9638-95529059138a`：APPROVE，无需 Terra。
- 页面节奏 follow-up：APPROVE；首帧 `_relayout()` follow-up `019ff59a-996b-76e0-adbf-48a8633392a1`：APPROVE。
- 独立代码 reviewer `019ff595-c839-7311-8c47-1a4006951912`：无 Critical/Required，`APPROVE WITH ADVISORIES`；其文档同步和首帧风险建议已处理。
- 最终代码 reviewer `019ff5a0-c897-7791-8c69-aeb9ffb23b47`：无 Critical/Required，`APPROVE WITH ADVISORIES`；确认最终 `_relayout()` 首帧与 resize 幂等。
- 独立简化 reviewer `019ff595-cb11-7370-b84c-6d088909da57` 与最终简化 reviewer `019ff5a0-cb7f-7fe3-aee0-f0c5a25920bc`：均无必须简化项；主题双层 selector 与薄节奏契约均保留。

## 非破坏性验证

- `compileall`：pass。
- Ruff：pass。
- `scripts/check.ps1`：pass；182 个 Python 文件均 <=1000 行，3 主题、22 semantic token、19 selector，legacy QSS literal=0。
- Qt offscreen 首帧/resize：520/640/720/980/1180 无卡片重叠、无同格坐标；反向 resize 幂等；命令 content top=0；accessible description 非空。
- 三主题 `star_trail` / `moonlit_ocean` / `sakura_night`：window/base palette 非白色；hide/show/close pass。
- 共享 MotionController：10.2 秒 active 样本 1224 frames，120.000Hz；无新增 timer、QPropertyAnimation 或 per-widget clock。
- onefile：`scripts/package.ps1 -Mode onefile -SourceRevision local-arch-123` 成功，provenance verify pass。

## 交付包

canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
根目录：`SerialForge.exe`、`SerialForge-latest.exe`  
三者均为 `48,047,222` bytes，SHA-256：`803114A03FA0FBEBF440005A1C79C6970FAB85C0863911CA2CF7B87DDCCB8054`。  
archive listing SHA-256：`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`。  
签名：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`。

## 未运行项目

真实 Windows GUI/HIDPI、显示器实际 120fps、EXE 启动/关闭、真实串口/网络/BLE/RTT、OTA/AES 传输与硬件验收未运行；offscreen 结果不替代这些验收。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
