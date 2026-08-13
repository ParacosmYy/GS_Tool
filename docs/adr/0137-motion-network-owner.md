# ADR-0137：动效降噪与网络 endpoint builder owner

- 状态：Accepted for ARCH-86 / UI-1.159
- 日期：2026-08-12
- 范围：`presentation/widgets.py`、`presentation/controllers/connection_builder.py`、
  `presentation/controllers/network_builder.py`、`presentation/controllers/connection_primitives.py`

## 背景

头部 signal rail 在固定的 148×34 装饰面中叠加了密集网格、5 个移动点、trail、scan line 和多个
sparkle，视觉层级过多；连接 builder 同时持有 shell、UART、网络、RTT 和 BLE 组合逻辑，继续增长
会让 runtime wiring 与控件构造互相牵连。用户希望动画保持 120Hz 目标且组件有更多留白。

## 决策

1. `MotionController` 不变为唯一共享时钟：`TARGET_HZ=120`、`PreciseTimer`、8ms scheduler target、
   elapsed phase 和完整 lifecycle gate 全部保留。
2. `SignalFieldWidget` 只做 presentation 绘制，将移动点减为 3 个按 `travel_width / 3` 均匀分布，
   导引线/网格/sparkle 降为轻量表达，不创建 local timer、状态源或业务依赖。
3. 新建 `network_builder.py` 作为 TCP Client/Server、UDP、RTT endpoint surface 的唯一构造 owner，
   原样返回 `NetworkControlBindings`，继续设置既有 window staged-compatibility 引用和 peer signal。
4. 新建 `connection_primitives.py` 提供无状态 label、bounded combo、responsive hint、section label
   helper；`connection_builder.py` 只负责 connection shell/UART/BLE 组合、root 插入和 endpoint callback
   wiring。
5. 网络 grid 间距调整为横向 12px、纵向 10px；通过留白缓解拥挤，不删除字段、不新增 nested scroll、
   backend、socket、Telnet、J-Link、线程或硬件动作。

## 不变量与风险

- `NetworkControlBindings` 27 个字段、默认值、范围、`itemData`、allowlist/LAN gate、peer/endpoint
  signal、Tab/accessibility contract 不变。
- `frame_changed(float, bool)`、`SignalFieldWidget.set_frame()/stop()`、pause/reduced-motion、
  hide/show/close 生命周期不变。
- 8ms 整数 Qt scheduler 名义上约 125Hz；本文和产品文案只称 120Hz target/约 120Hz，不宣称精确
  显示器刷新率。
- 本轮只涉及 Python/PySide6 presentation；无 embedded C/C++、MCU、SDK、RTOS、driver、OTA 实现或
  target hardware change，public first-party vendor source applicability 为 N/A。

## 验证与复核

已完成 `ruff`、`compileall`、`scripts/check.ps1`、presentation import contract、源文件行数门禁和
onefile provenance verify。
ARCH-86 Luna/max 架构师后续只读结果为 `PASS / Required=0`，确认唯一计时器、三点等距轨道、binding
字段和两类信号连接无遗漏。独立 code-review/simplification Luna/max 调用在服务窗口内超时并关闭，
未伪造 PASS；父代理完成 independent fresh-pass，Required=0，简化评估为保留单一时钟、无状态
primitives、原 typed binding/accessor 和 signal owner。GUI/EXE 启动、三主题几何、真实帧率、硬件/HIL
验收未运行，已分别标记为 not-run。
