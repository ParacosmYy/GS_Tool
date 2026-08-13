# ADR-0156：工作区滚动提示同时表达位置与动作

日期：2026-08-12

状态：已采用（ARCH-105 / UI-1.178）

## 背景

长配置页的 route strip 原先只显示“向下查看”“上下滚动”或“返回顶部”，用户需要结合
滚动条位置才能判断自己处于页面哪个阶段。协议/遥测和扩展工具站的内容本身没有几何重叠，
但工作区的长页面缺少稳定的空间定位感。

## 决策

- 继续由 `WorkspaceScrollHint` 观察当前 `QScrollArea` 的原生 vertical scrollbar；不新增
  滚动状态源、计时器、section 计数或业务字段。
- `top/middle/bottom/complete` 状态文案升级为“位置 · 下一步动作”：
  `顶部 · ↓ 向下查看`、`中段 · ↕ 上下滚动`、`底部 · ↑ 返回顶部`、
  `全显 · 内容已全部显示`。
- 可访问名称从“工作区滚动提示”明确为“工作区滚动位置”，动态 accessible description 继续
  说明当前可执行方向。
- 稳定 controls stylesheet 只将提示字重提升到 600；主题颜色仍由既有四态 selector 和
  `ThemeSpec` token 提供，route strip 高度、宽度契约、滚动 owner、Tab/焦点路径不变。

## 验证与取舍

真实 Qt offscreen 三主题、980×720/1240×820、四个 workspace 的 top/middle/bottom 状态均与
原生 scrollbar value/max 正确对应；横向滚动最大值均为 0，提示 geometry 为 112×22，最长文案
字体测量仍小于 112px。focus 模式、accessibleName/Description 和现有 120Hz shared clock
生命周期未受影响。截图位于 `C:\Users\Gs\AppData\Local\Temp\serialforge-arch105-scroll-*.png`。

本轮为 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A；
没有固件、MCU、BSP/HAL/RTOS、OTA 实现或 C/C++ 修改，不作 MISRA、ISO 26262、ASIL、ASPICE
或认证声明。架构师调用在等待窗口内超时关闭，未形成外部结论；父代理完成行为保持的五轴
review 与简化评估。
