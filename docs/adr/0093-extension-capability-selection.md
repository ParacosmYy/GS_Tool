# ADR-0093：扩展工具站能力卡选择与详情投影

日期：2026-08-12

状态：已采用（supersedes ADR-0092 的静态卡片交互限制）

## 背景

扩展工具站已有 3 个 OTA 传输、2 个 OTA 安全和 2 个 RTT/J-Link 能力 DTO。旧 UI-1.120 将卡片保持为
静态 `QFrame`/NoFocus，用户只能看到摘要，无法通过键盘逐项检查能力边界；同时没有一个稳定的详情
区域承载 key、group、reference、state 等可审计事实。用户要求减少组件挤压，并为后续嵌入式调试工具站
保留清晰的高内聚扩展边界。

## 决策

- 保持 `embedded_extension_panel.py` 为唯一 presentation 组装 owner。它消费 immutable DTO，并在
  panel 内部维护单一选择状态；`ExtensionPanelWidgets` 不向 workspace 暴露详情内部控件。
- 每张能力卡改为完整 `QPushButton` 语义控件：卡片只有一个 Tab stop，click、focus、Enter、Space
  共用同一 bounded selection path。选择只改变 selected presentation 和详情，不触发 backend、设备、
  OTA、AES、RTT 或 J-Link 动作。
- 新增 `ExtensionCapabilityDetail` 作为 stateless DTO→view 叶子，固定展示 title、group/reference、
  raw key/group/reference/state、summary、boundary 和“不执行” guard。它复用外层 scroll page，不创建
  nested scroll、splitter、sticky owner、timer 或线程。
- extension 专属 QSS 放入 `theme_stylesheet_extension.py`；composer 固定为
  `BASE_STYLESHEET + EXTENSION_STYLESHEET + CONTROLS_STYLESHEET`，三主题 shell 对称覆盖 card/detail。

## 结果

能力卡从“只读但不可检查”变为“只读且键盘可检查”，详情和 raw identity 的 owner 清晰，后续接入真实
工具仍需要另行补齐型号、授权、公开一手资料、失败恢复和硬件验收证据；当前不会因为 UI 存在而暗示
OTA 升级、解密、签名激活或探针控制已经可用。

## 验证

- `uv run ruff check src`：通过。
- `uv run python -m compileall -q src`：通过。
- `scripts/check.ps1`：通过，169 个源文件均不超过 1000 行，3 themes、22 semantic tokens、19 selectors。
- Qt offscreen 真实组合根：7 cards、click/focus/Enter/Space、DTO unchanged、三主题 horizontal maximum=0、
  exact-white=0、reduced-motion、close 清理均通过；实际字体为 `Microsoft YaHei UI`。
- 截图：`C:\Users\Gs\AppData\Local\Temp\serialforge_arch83_extension_0.png`（980×720）已完成人工视觉复核。
- 未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产；未连接、刷写、部署或操作目标硬件。

嵌入式 C/C++ 适用性：N/A。本轮只有 Python/PySide6 presentation 改动，没有 firmware/MCU/BSP/HAL/RTOS/
bootloader/Flash/OTA backend 修改；不声明厂商要求、MISRA、ISO 26262 或任何认证合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
