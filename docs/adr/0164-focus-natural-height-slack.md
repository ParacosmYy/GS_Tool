# ADR-0164：专注工作区自然高度与透明 slack

## 状态

已接受（ARCH-113 / UI-1.186，2026-08-12）

## 背景

专注设置页在窗口尺寸、当前 route 和切换时机不同的情况下，原先可能被通用 shell 高度策略拉出
大块无语义空白，或被下方实时/终端/发送 surface 挤压。专注模式的目标是让配置组件完整、可读，
同时不改变后台连接、记录、终端 ingestion 和共享 120Hz 动效时钟。

## 决策

由 `src/serialforge/presentation/workspace_focus_transition.py` 单独拥有专注页静态布局结算：

1. focus 隐藏 live observation 与 send surface；
2. 复用已有透明 `terminalSurface` 作为 root layout slack，不新增根级 spacer；
3. 根据当前 `QScrollArea` 内容 `minimumSizeHint()` 加 tabs/shell chrome 计算目标高度；
4. 目标高度可容纳时固定为自然高度，放不下时保留原生 vertical scroll；
5. 用快照恢复 terminal 子控件 hidden 语义、父 surface 状态、鼠标透明、size policy 和 min/max；
6. tab route 与 resize 只调用公开的 `refresh_workspace_focus_layout()`，不复制公式、不重启动效。

布局先静态收敛，focus/overview 的现有 `MotionDrivenAnimationGroup` 仍只负责 opacity。组合根、业务
状态、ViewModel、worker、scroll owner、`MotionController`、timer、QSS 和 OTA/AES/RTT/J-Link
边界不变。

## 被否决的替代方案

- 使用固定 shell 高度：无法同时覆盖 connection、command 和 extension 的自然内容高度。
- 修改 bootstrap/root layout 顺序或新增根级 spacer：扩大组合根风险并引入新的伸缩 owner。
- 用 `setGeometry()`/`move()` 手工摆放：绕过 Qt layout contract，resize、主题和 DPI 下更脆弱。
- 继续逐帧动画 `maximumHeight`：会让 QTabWidget viewport 每帧挤压内容，重新引入用户报告的拥挤问题。
- 直接销毁或永久隐藏 terminal 内容：破坏 overview 恢复、空态状态和终端 presentation contract。

## 验证

三主题（`star_trail`、`moonlit_ocean`、`sakura_night`）× `980×720`、`1180×780`、`1240×820` × 四
workspace，共 36 个真实 Qt offscreen cases 通过：focus/overview、重复 `True/False`、before-show
focus、route 切换、resize、暂停/隐藏/恢复/关闭、focus terminal hidden、overview terminal restored、
live/send 高度恢复、横向 `HMAX=0`、唯一 MotionController 均通过。1240 connection shell/page 为
`536/462px` 且 `vmax=0`；command 为 `331/257px` 且 `vmax=58`；extension 为 `485/411px` 且
`vmax=623`。静态 check、compileall、ruff、行数和 theme audit 通过。

架构师 Hubble、Pasteur、Russell 的方案/约束结论已纳入实现；Kant 的 refresh wiring 调用与独立
reviewer Pauli 均在等待窗口超时关闭，未形成外部结论。父代理完成 correctness、architecture、
security、performance、readability 五轴 review 与 behavior-preserving simplification assessment。
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A；真实 GUI/HIDPI、显示器 FPS、
EXE startup、硬件、签名和 OTA/RTT 实连不在本轮授权范围内。
