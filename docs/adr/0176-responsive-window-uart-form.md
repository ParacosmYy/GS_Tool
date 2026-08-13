# ADR-0176：窄窗口响应式连接带与 UART 参数表单

## 状态

已接受（ARCH-125 / UI-1.198，2026-08-12）

## 背景

窗口原先存在 `980×720` 的硬最小尺寸；即使去掉部分上层纵向 stretch，连接 shell 和 UART 字段仍会在较窄宽度挤成横向长条。
用户需要在不牺牲可读性、主题完整性和既有控件行为的前提下缩小窗口，并为后续嵌入式调试工具站保留清晰的模块边界。

## 决策

- `bootstrap.py` 不再设置固定窗口最小尺寸，只保留启动时的 `1240×820` 初始偏好；自然最低宽度由内容 sizing contract 决定。
- `_ResponsiveConnectionBand` 成为连接/预设顶部 shell 的唯一 presentation geometry owner。它接收 builder 已创建的九个 widget，按自身
  `minimumSizeHint()/sizeHint()`、layout spacing 和 margins 选择 `REGULAR`、`COMPACT`、`NARROW_COMPACT`，只改变布局位置，不复制业务状态或
  重新创建控件。每次 reflow 先清理当前 stretch，再由 mode placer 设置当前 mode 所需的 stretch。
- `ResponsiveUartForm` 成为 UART 字段的独立 geometry owner。builder 使用 `build_labeled_field()` 创建十个 wrapper 并注入；owner 只管理
  `REGULAR` 三行宽屏布局、`COMPACT` 成对字段布局和 `NARROW` 单列布局。owner 不拥有 raw control、UART DTO、signals、transport、timer 或
  runtime binding。
- FontChange、StyleChange、LayoutRequest 和 resize 都触发 sizing invalidation；模式未变化时只 invalidate/activate，不 churn widget identity。
- 不新增 QSS selector、白色 fallback、滚动区域、动画时钟或业务接口；保留唯一共享 `MotionController` 与既有 `120Hz` scheduler contract。

## 边界与非目标

本 ADR 只覆盖 Python/PySide6 presentation geometry。它不实现新的 OTA、AES、XMODEM/YMODEM/TFTP、RTT 或 J-Link 设备能力，也不改变 UART/TCP/UDP/BLE
runtime transport、ViewModel、DTO、bindings、signals、Tab order、accessibility 或业务状态。`main.py`/composition root 仍只负责应用启动与依赖组合。
本轮无 embedded C/C++、固件或硬件改动，public-vendor-source applicability=N/A。

## 评审与验证

架构师 `019ff5c9-8eff-7cd3-9c2f-ab0a40ec9ef0` 对 owner、边界、sizing contract 与最终循环简化均 APPROVE。独立审查
`019ff5df-ef65-7f00-9b30-03c62fad4be9` 最终为 `APPROVE WITH ADVISORIES`（Critical/Required=0）；简化审查
`019ff5df-efac-7453-96b5-f9d61b07c0ff` 最终为 `APPROVE WITH ADVISORIES`（无必须简化项）。

最终门禁应记录 compileall、Ruff、`scripts/check.ps1`、source-limit、theme audit，以及三主题下
`520/546/560/640/768/900/980/1180px` 的真实 Qt offscreen 几何、FontChange/StyleChange/LayoutRequest、hide/show/close 与生命周期证据。
offscreen 证据不等于真实 Windows GUI/HIDPI、显示器实际 120fps、EXE startup、硬件连接或正式发行验收。
