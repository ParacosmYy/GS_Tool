# ARCH-127 / UI-1.200 UART 参数表单列级 sizing

日期：2026-08-12  
范围：`src/serialforge/presentation/responsive_uart_form.py`

## 本轮结果

UART owner 现在以字段自身 `minimumWidth`、`minimumSizeHint()`、`sizeHint()` 推导 hard/preferred width。regular 使用四个语义列，
compact 使用两列，narrow 使用单列；water-fill 规则确定性分配左右列对的剩余空间。`minimumSizeHint()` 不再把 regular preferred width 伪装成
窗口硬下限，builder 继续拥有控件、signals、bindings 与 UART runtime wiring。

## 证据

- 三主题 `star_trail`、`moonlit_ocean`、`sakura_night`。
- 宽度 `546/547/560/600/640/768/900/1120/1240px`。
- 546/560 为 narrow，600/640/768 为 compact，900 以上为 regular。
- 每个字段宽度不小于自身 hard minimum；字段无 overlap；FontChange、StyleChange、LayoutRequest、resize 往返、focus/identity 与生命周期通过。
- 文件行数 332，低于 1000 行门禁。

架构师 `019ff646-9603-70e3-9d75-5e40bb286b44` 修订后 `APPROVE`；简化评估
`019ff66c-290b-7cd0-9d06-736a955dd567` 无必须项。独立审查与本轮 ARCH-128 共用最终复核记录；本轮无 embedded C/C++，
public-vendor-source applicability=N/A。

## 尚待交付

ARCH-127 与 ARCH-128 合并执行 compileall、Ruff、主题/源码行数检查、onefile 构建和根目录 `SerialForge.exe`/`SerialForge-latest.exe` 覆盖。

