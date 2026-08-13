# ADR 0030：连接快速配置的自定义选中态与语义上下文

状态：已接受（2026-08-10）  
范围：`presentation/connection_preset_surface.py`、连接 builder/controller、连接 preset selector 主题规则和主题静态审计

## 背景

连接快速配置已经有 `customSelected` 动态属性和“自定义 · 名称”可见前缀，但主题没有消费该状态，导致内置与自定义配置在关闭下拉框后缺少稳定的视觉区分。
同时，ComboBox 的 `AccessibleDescription` 固定写成“内置连接快速配置”，选择自定义项后 controller 又把 tooltip 覆盖为纯 preset 描述，视觉状态与辅助技术语义会分叉。

## 决策

- 复用 `presentation/property_refresh.py` 的唯一动态属性刷新原语，允许 Qt-compatible 的 `str | bool` 值；保持值未变化时跳过 polish，变化时执行
  `setProperty → unpolish/polish → update`。
- 由 `connection_preset_surface.py` 集中拥有 `is_custom_connection_preset()` 和
  `update_connection_preset_context()`：refresh、用户选择两条路径都同步 `customSelected`、tooltip 和
  `AccessibleDescription`。空态、内置、自定义三态使用不同的文字语义；可见“自定义 ·”前缀仍是颜色之外的识别依据。
- 默认主题与三个主题 override 都消费
  `QComboBox#connectionPresetCombo[customSelected="true"]`，补齐 normal/hover/focus/disabled 状态；disabled 显式回退到 disabled token，不能残留自定义底色。
- 将该 selector 加入 `scripts/check_theme_tokens.py` 的必需 selector 集合，防止未来三套主题缺失自定义选中态却通过静态门禁。
- 连接 preset 仍然只填充表单，不自动连接、不保存密钥/设备句柄/BLE 身份；本 ADR 不引入 dirty 表单状态，也不把 `customSelected` 提升为 domain/application 事实源。

## 被拒方案

### 在 controller 中分别 setProperty、setToolTip 和 AccessibleDescription

拒绝：保存、删除、初始化和用户选择很容易产生不同语义，且 controller 会复制 QSS 生命周期细节；上下文应由 presentation surface 统一维护。

### 自绘 ComboBox 或自定义 popup

拒绝：会重做键盘、焦点、popup 和无障碍契约；原生 ComboBox 已有 bounded 数据和主题化 popup，只需补齐语义 selector。

### 新增常驻动画或伪造“表单已匹配”状态

拒绝：选中 preset 不是连接会话状态，手动修改字段后的 dirty 语义需要独立产品定义；本切片不新增 timer、业务字段或状态源。

## 后果

- 用户在不展开下拉框时可以看到自定义前缀和独立主题底色；键盘/读屏用户可获得当前内置、自定义或未选择语义。
- 动态 QSS 状态和删除按钮 gate 仍共享现有 presentation selection 事实；后续如需进一步收窄，应让删除按钮从 typed `UserRole` selector 推导，而不是把 QSS 属性当作业务事实源。
- 未新增颜色 token，复用已有主题语义 palette；文字前缀和辅助描述保证 custom 识别不依赖颜色。

## 验证与限制

- `scripts/check.ps1`：126 个 Python 源文件均不超过 1000 行；3 主题、22 semantic tokens、12 必需 selectors、Ruff 通过。
- `UI143_CONTEXT`：star_trail、moonlit_ocean、sakura_night 的空态/内置/自定义 description、controller selection、视觉变化、disabled 回退通过；custom/disabled near-white 均为 0。
- `UI143_IMPORT modules=125 qapplication_instance=False`；`UI143_COMPILE pass`。
- 截图：`build/ui_review_ui143_preset_star_trail.png`、`build/ui_review_ui143_preset_moonlit_ocean.png`、
  `build/ui_review_ui143_preset_sakura_night.png`。offscreen 环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。
- 未运行持续 GUI、EXE 启动、真实键盘/Tab/读屏/HIDPI、UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件或签名发布验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式适用性：本 ADR 仅涉及 Python/PySide6 presentation/QSS，不涉及 MCU、BSP/HAL/C++、RTOS 或厂商资料；不宣称 MISRA、ISO 26262、WCAG、认证或硬件合规。
