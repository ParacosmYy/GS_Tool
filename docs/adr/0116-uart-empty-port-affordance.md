# ADR-0116：UART 空端口输入引导

## Status

Accepted

## Date

2026-08-11

## Context

UART 端口列表为空时，端口控件仍允许用户输入 `COMx`，但只设置
`QComboBox.placeholderText` 在 Qt 的可编辑组合框中不足以保证提示出现在内部编辑框。
真实组合根验证发现组合框 placeholder 已存在，而 `QLineEdit` placeholder 为空，用户看到的输入区
因此像未完成或不可用的控件。

## Decision

在 `presentation/controllers/connection_builder.py` 的 UART 端口组装边界定义一个本地
`port_placeholder` 文案，同时设置 `QComboBox` 和其内部 `QLineEdit` 的 placeholder。端口 combo
继续保持唯一的可编辑 selector；placeholder 只提供“未发现端口 / 点击刷新 / 可输入 COMx”的引导，
不写入 endpoint、不会自动连接，也不改变 `currentText()`、连接 gate 或 `connection_runtime.py`。

## Alternatives Considered

### 只设置 QComboBox.placeholderText

拒绝：Qt 可编辑模式下内部 `QLineEdit` 不会自动继承该提示，真实运行时仍会留下空白输入区。

### 禁止手动输入并只显示刷新按钮

拒绝：Windows 环境中的端口枚举可能暂时为空，且现有产品契约允许用户输入已知的 `COMx`。

### 用状态栏或错误栏替代控件内引导

拒绝：这会把局部表单 affordance 扩散到全局状态区域，降低输入路径的就地可发现性。

## Consequences

- 无端口时输入区仍提供明确、主题化的下一步，用户无需先理解内部端口枚举机制。
- `COM99` 等手动输入仍按原有 `currentText()` 路径进入连接校验；不增加业务状态、timer、线程、I/O 或依赖。
- 组合框和内部编辑框共享同一语义文案，后续主题只需沿用既有控件 token，不新增白色回退规则。

## Verification

真实组合根覆盖星轨霓虹、月影深海、樱雾夜航三主题及 980×680/1180×780 两种尺寸：
QComboBox 与内部 QLineEdit placeholder 均为 `未发现端口 · 点击刷新或输入 COMx`，`COM99`
手动输入/清空保持兼容，当前页 horizontal maximum=0，截图 exact-white=0；视觉证据为
`build/ui_review_ui129_uart_empty_port.png`。静态门禁、compileall、Ruff 和 onefile provenance
随后通过。架构师线程 `019fed79-c377-7823-a89e-c1c3127cf77d`、
`019fed7b-a8ca-7540-b9d7-68257c3985f5`、`019fed7d-f207-7342-9cd5-a9f00a3ca086`
均在限定窗口内超时，未计为独立通过；父代理完成 owner、兼容性、生命周期、五轴质量和简化评估。
未修改嵌入式 C/C++，public vendor applicability=N/A，真实硬件验证未运行。
