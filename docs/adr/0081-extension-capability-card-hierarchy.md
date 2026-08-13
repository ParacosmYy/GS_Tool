# ADR-0081：扩展 capability card 的语义层级

日期：2026-08-10  
状态：Accepted  
范围：`presentation/theme_stylesheet_base.py`、扩展工具站 capability card

## 背景

UI-1.90 引入了只读的“扩展 / 工具站”页面。卡片标题此前复用了通用
`QLabel[role="status"]` 规则，导致标题也呈现为状态胶囊；能力状态本身又缺少
稳定、可区分的 info/history 语义。结果是标题和状态的视觉职责混在一起，且三主题
无法从统一 token 规则获得清晰回退。

## 决策

- 标题使用 `QLabel#extensionCapabilityTitle`，回到正文 `TEXT` 层级，透明背景、无边框、无胶囊 padding。
- 状态使用 `QLabel#extensionCapabilityState`，默认 `contract_only` 使用 `ACCENT_BLUE`、`INFO_SURFACE` 和 `INFO_BORDER`。
- `state="attach_only"` 使用 `ACCENT_PURPLE`、`HISTORY_SURFACE` 和 `HISTORY_BORDER`，表达“可附着但不由 SerialForge 托管”的历史/外部桥接边界。
- 只复用已有 `ThemeSpec` token；不增加 DTO 字段、颜色字面量、业务状态源、动画时钟或交互动作。

## 依赖方向与边界

`application.extension_capabilities` 继续提供不可变 capability DTO；
`presentation.embedded_extension_panel` 继续只负责渲染和无障碍文本；稳定 QSS
仍由 presentation theme owner 生成。OTA、AES、RTT、J-Link 的执行 adapter、密钥、
设备句柄和 vendor SDK 不进入本次切片。

## 验证与风险

三主题 Qt offscreen hierarchy vector 通过：7 个标题、7 个状态、两种 state selector
均存在并可 polish。`scripts/check.ps1`、compileall、ruff 通过，源码文件均不超过 1000 行。
未启动可见 GUI/EXE，未运行 HIDPI、读屏、真实 OTA/AES/RTT/J-Link 或硬件验收；这些项目
需要用户授权和目标资料。Python/PySide6 presentation 变更，嵌入式 C/C++ 适用性：N/A。
