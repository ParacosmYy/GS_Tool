# ADR 0029：批量编辑器原生表头与 SpinBox 子控件主题回退

状态：已接受（2026-08-10）  
范围：`presentation/command_batch_editor.py`、`theme_stylesheet_controls.py`、`theme_variant_controls.py`

## 背景

批量命令编辑器在真实 `MainWindow` 主题下仍出现白色系统 palette 回退：表格左侧的 vertical header 形成一整列白色；
延时 `QSpinBox` 右侧的 up/down button 区域还留下 40 个近白像素。两者都不是业务内容，但会破坏深色二次元主题的一致性。

## 决策

- 批量编辑器的步骤序号以已有横向“步骤”列为唯一可见序号来源；隐藏冗余的
  `QTableWidget.verticalHeader()`，不改变 row selection、`currentCellChanged`、步骤数据、无障碍名称/description 或保存逻辑。
- 在默认 stylesheet 与三套主题 override 中显式覆盖 `QSpinBox/QDoubleSpinBox` 的 `::up-button`、`::down-button`、
  `::up-arrow`、`::down-arrow`：背景、hover、disabled 和语义 token 箭头均由 QSS 提供；保留原生 stepper、键盘和焦点行为。
- 规则只匹配 QSpinBox/QDoubleSpinBox，不修改 QComboBox popup/dropdown；不引入图片、字体、QRC、业务字段或新 timer。

## 被拒方案

### 只给表格第一列加深色背景

拒绝：白色区域属于 vertical header 子控件，不是数据列；覆盖数据列不能稳定修复 header geometry，也会把冗余序号继续暴露给用户。

### 关闭 SpinBox 上下按钮

拒绝：会减少用户可配置性并破坏现有鼠标 stepper；应该保留原生能力，只替换子控件视觉层。

### 使用外部箭头图片或字体图标

拒绝：增加资源部署、HIDPI、主题切换和打包边界；QSS 三角箭头足以表达 bounded 的增减操作。

## 后果

- 批量编辑器不再出现整列白色或 SpinBox 子控件白色回退，且步骤序号更清晰。
- 主题规则略微增加 native-control QSS selector 数量，但职责仍集中在 controls stylesheet；业务文件只增加一行结构性设置。
- Windows 原生运行时仍需 GUI/HIDPI/读屏验收；offscreen 缺失 PySide6 fonts directory 的提示不作为字体结论。

## 验证与限制

- `scripts/check.ps1`：126 个 Python 源文件均不超过 1000 行，theme token audit 与 Ruff 通过。
- 真实 `MainWindow` parent 下的 `CommandBatchEditorDialog`：star_trail、moonlit_ocean、sakura_night 三主题 near-white pixel count 均为 0；
  vertical header 为隐藏，horizontal steps header 保持可见，表格几何 738×259。
- 未运行持续 GUI、EXE 启动、HIDPI/读屏、真实传输、OTA/RTT/J-Link、硬件或签名发布验收；未创建、修改或运行 unit test、mock、
  fixture、harness 或 test-only 资产。

嵌入式适用性：本 ADR 仅涉及 Python/PySide6 presentation/QSS，不涉及 MCU、BSP/HAL/C++、RTOS 或厂商资料；不宣称
MISRA、ISO 26262、WCAG、认证或硬件合规。
