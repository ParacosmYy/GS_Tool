# ADR-0165：主题工作区根布局伸缩权 owner

## 状态

已接受（ARCH-114 / UI-1.187，2026-08-12）

## 背景

上一轮已让 focus 页面按内容自然收敛，但根布局仍可能把可用高度分配给错误的 sibling，造成组件被挤压、
路由条与内容拥挤或出现一整行无语义空白。目标是让主题 shell 成为 focus 模式的唯一高度 owner，同时
保留 overview 的原始终端与下方 surface 语义。

## 决策

由 `workspace_focus_transition.py` 维护一个临时 `_FocusLayoutSnapshot`：

1. 进入 focus 前保存 root stretch、shell/tabs policy 与 min/max、tabs/route stretch、terminal parent/child
   hidden、鼠标透明和 terminal policy/min/max；
2. focus 中设置 root `shell=1, terminal=0`，隐藏 terminal slot 与 live/send surface；
3. shell 内 tabs 取得 stretch `1`，route strip 取得 stretch `0`，固定路由条的 31px geometry；
4. 页面几何立即结算，视觉上只使用共享 MotionController 的 opacity 过渡；
5. overview 按统一恢复路径还原全部快照，布局 activate 完成后清空哨兵。

这样不改变 root child 顺序、业务/连接状态、scroll owner、120Hz scheduler 或 OTA/AES/RTT/J-Link 模块
边界；长页面仍由现有 QScrollArea 原生滚动处理。

## 被否决的替代方案

- 继续逐帧动画 `maximumHeight`：会在每帧压缩 viewport，直接复现组件挤在一起的问题；
- 新增根级 spacer 或重排 composition root：引入第二个 stretch owner，扩大生命周期风险；
- 用 `setGeometry()`/`move()` 固定 shell：破坏 Qt layout contract，主题、resize 和 DPI 下不可维护；
- 永久隐藏 terminal 子控件：会丢失 overview 的原始 hidden 语义与恢复能力。

## 验证与审查

三主题×三尺寸×四 workspace 的 36 个真实 Qt offscreen cases、72 个 focus/restore 状态检查通过；
lifecycle matrix 118 项检查通过。静态 source-limit、theme audit、check、compileall、ruff 均通过。
架构师 `019ff489-8b27-77a2-b059-bc160805c0fb` 的正式方案结论为 conditional pass；架构师
`019ff497-1040-7721-8aa4-865f176e9e1d` 确认 `_set_terminal_spacer` 应使用 `bool` 契约并规范失败返回。
本轮新增独立 reviewer `019ff499-0005-76b3-9e7c-e042c2777518` 在等待窗口内超时关闭，未形成外部结论；
父代理完成 correctness、architecture、performance、lifecycle、security、readability 五轴 review 与
behavior-preserving simplification assessment。无嵌入式 C/C++ 改动，public-vendor-source applicability
为 N/A；真实 GUI/HIDPI、EXE startup、硬件、签名和 OTA/RTT 实连不在本轮授权范围。

## 交付

`local-arch-114` onefile 已生成并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical、
root、root-latest 均为 `48,026,927` bytes，SHA-256 为
`966C16A74D04454CE8780F424E19D99773E3E39B1CBE42A0A5490812CB605F2C`。archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过；签名
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
