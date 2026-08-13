# ADR-0178：UART 参数表单列级 sizing contract

状态：Accepted  
日期：2026-08-12  
范围：`src/serialforge/presentation/responsive_uart_form.py`

## 背景

窄窗口下，UART 参数表单的四列布局虽然已经进入响应式 owner，但右侧“线路控制”等字段仍可能被压到小于自身
`sizeHint()` 的宽度，形成文字挤压。窗口最低宽度不能依赖固定魔法值，也不能把宽屏 preferred width 伪装成窗口硬下限。

## 决策

- `ResponsiveUartForm` 继续是十个既有 labeled field wrapper 的唯一几何 owner；不创建 raw control，不接管 DTO、signals、连接状态、
  MotionController 或业务动作。
- 每个字段的 hard width 为 `m(f)=max(f.minimumWidth, f.minimumSizeHint.width)`；preferred width 为
  `p(f)=max(m(f), f.sizeHint.width)`。
- `REGULAR` 使用四个语义列：
  `data_bits/read_timeout`、`parity/write_timeout`、`stop_bits/inter_byte_timeout`、
  `flow_control/line_controls`。列宽先取每列 hard max，再把可用宽度按确定性的 water-fill 规则分配给左右列对。
- `COMPACT` 使用两列：左列承载 `port/data_bits/stop_bits/read_timeout/inter_byte_timeout` 的最大 hard width，右列承载
  `baud/parity/flow_control/write_timeout/line_controls` 的最大 hard width；`NARROW` 使用单列。
- 模式判断先比较 regular preferred threshold，再比较 compact hard threshold；任何 mode 变化或字体/style/layout 事件都在同一 owner
  内清理并重新设置 column minimum/stretch，不重建控件、不跨 layout 挂载。
- `minimumSizeHint()` 只暴露当前可进入的 hard contract；`sizeHint()` 才表达 regular preferred width。

## 结果与证据

三主题 `star_trail`、`moonlit_ocean`、`sakura_night` 与 `546/547/560/600/640/768/900/1120/1240px` 离屏矩阵通过：
546/560 进入 `NARROW`，600/640/768 进入 `COMPACT`，900 以上进入 `REGULAR`；所有字段宽度不小于自身 hard minimum，字段无重叠，
线路控制在 regular 下保留完整 hard width。FontChange、StyleChange、LayoutRequest、往返 resize、焦点/identity 和生命周期检查通过。
相关文件均不超过 1000 行。

架构师 `019ff646-9603-70e3-9d75-5e40bb286b44` 最终 `APPROVE`（修订后）；独立代码审查与简化评估分别记录在本轮交接中，
无 Critical/Required 阻断、无必须简化项。Python/PySide6 presentation-only；embedded C/C++ public-vendor-source applicability=N/A。

