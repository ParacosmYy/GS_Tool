# ADR-0070：分析状态 rail 的静态终态 marker

日期：2026-08-10  
状态：Accepted for UI-1.83  
范围：`presentation/analysis_status_surface.py`

## 背景

Protocol、Component、Dataset、Curve 四个状态条已经共享 `AnalysisStatusLabel` 的文字、`state/source` properties 和 signal rail。`error`、`blocked`、`history` 目前只有节点亮度与颜色差异，用户在信息密集页面中不易快速确认状态类别。

## 决策

在既有 `paintEvent()` 底部 rail 内增加三种静态几何 marker：`error` 使用叉、`blocked` 使用双横栏、`history` 使用回退箭头。marker 位置由已有 active-node 映射决定，颜色由当前 `ThemeSpec` 语义 token 提供；文字和 accessibility 仍是权威语义。

`active`、`waiting`、`draft` 继续消费 shared frame；静态状态不依赖 frame。未知状态、`empty`、`idle`、reduced-motion、暂停、隐藏、最小化和关闭保持中性静态回退。

## 拒绝的方案

- 不新增状态 DTO 或通用 icon registry：当前 renderer 已有完整 bounded state 集合，新增抽象只会扩大耦合。
- 不为 marker 创建局部 `QTimer`：共享 MotionController 已负责生命周期，静态 marker 无需时钟。
- 不用颜色单独表达状态：每个 marker 有不同几何形状，文字和 AccessibleDescription 不变。

## 验证

静态检查、`compileall`、三主题 × 三状态真实组合根 offscreen render 均通过；主窗口未 `.show()`。完整 Windows GUI/HIDPI/读屏、真实设备/网络、OTA、签名和正式发行验收未运行，详见 [最新交接](../handoffs/current.md)。

## 评审与适用性

六个职责角色在源码修改前按项目约束调用，均在窗口内超时并关闭；独立质量复核在源码修改后调用，同样超时并关闭。父代理完成 correctness、readability/simplicity、architecture、security、performance 五轴复核；嵌入式 C/C++ 适用性为 N/A。
