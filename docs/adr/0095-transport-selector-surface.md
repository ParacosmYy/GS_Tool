# ADR-0095：传输方式选择器 semantic surface

- 状态：已接受
- 日期：2026-08-11
- 范围：`presentation/theme_stylesheet_base.py`、`presentation/theme_variant_shell.py`

## 背景

连接页的 `transportCombo` 是 UART、TCP Client/Server、UDP、BLE GATT 和 J-Link RTT 的唯一可访问选择器，旁边已有
`TransportModeSurface` glyph，但下拉本身仍使用普通控件的中性表面。重要的链路入口因此缺少与快速配置、连接状态 rail
相匹配的视觉层级，disabled/focus 状态也没有专属语义边界。

## 决策

在 presentation theme owner 中为 `QComboBox#transportCombo` 增加 base/variant 对称 selector：

- 默认使用既有 `info_surface` → `surface_input` 渐变、`info_border` 和强调字重；
- hover 使用 `accent_blue`，focus 使用既有 `focus`，disabled 使用既有 disabled semantic token；
- controller 继续创建 combo、维护 `TransportKind` itemData 和 signal；glyph 继续是只读装饰，不能成为第二状态源。

## 后果

传输方式作为连接页首要入口更容易被扫描，六种 transport 的数据 contract、不可编辑约束、accessibility、连接 gate 和
业务生命周期完全不变。样式仍集中在 theme owner，未来增加 transport 时复用同一 selector contract，不把 palette 或
状态判断散落到 builder/runtime。

## 验证与边界

真实组合根 Qt offscreen vector 覆盖三套主题、六种 `TransportKind`、`itemData`、不可编辑、enabled/disabled 和整窗
near-white pixel=0；静态门禁、compileall、ruff、provenance 与 onefile 打包通过。未显示 GUI、未启动 EXE、未连接真实
设备或执行 OTA/J-Link；本轮只有 Python/Qt presentation 代码，vendor public source applicability=N/A，不声明 MISRA/ISO/硬件合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
