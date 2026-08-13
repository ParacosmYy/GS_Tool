# ADR-0072：稳定 QSS 近白文字/选中色 token 化

日期：2026-08-10  
状态：Accepted for UI-1.85  
范围：`presentation/theme_stylesheet_base.py`、`presentation/theme_stylesheet_controls.py`

## 背景

主题系统已经通过 `ThemeSpec` 和 variant stylesheet 支持星轨霓虹、月影深海、樱雾夜航，但 stable QSS 仍有八处直接写死的 `#fff…` 文字或选中色。它们不是白背景，却会让非默认主题继承不属于自身的近白高光，削弱主题一致性和后续维护可读性。

## 决策

默认文字使用已有 `TEXT`，选中/高亮文字使用已有 `SELECTION_TEXT`。替换范围为 station section、line edit/plain text/combo selection、combo item hover/selected、menu selected、workspace tab selected、table selection。ThemeSpec 字段、variant renderer、selector、焦点、selection background、文字/accessibility 和业务状态均不变。

## 拒绝的方案

- 不新增 `NEAR_WHITE` token：现有 `TEXT`/`SELECTION_TEXT` 已表达语义，新增 token 会扩大主题 API。
- 不把所有稳定 QSS 颜色一次性重写：本轮只处理近白文字/选中色，避免混入无关视觉重构。
- 不修改 selection background 或控件交互：问题是颜色来源一致性，不是交互行为。

## 验证

稳定模板近白 literal count=0；三主题 stylesheet 无白背景 fallback；checkbox/combo/table/tab 内存渲染和 selection token 解析通过，详见 [最新交接](../handoffs/current.md)。

## 评审与适用性

六个职责角色在源码修改前按项目约束调用，均在窗口内超时并关闭；独立质量复核在源码修改后调用，同样超时并关闭。父代理完成 correctness、readability/simplicity、architecture、security、performance 五轴复核；嵌入式 C/C++ 适用性为 N/A。
