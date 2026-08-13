# ADR-0114：工作区专注设置模式过渡

## Status

Accepted

## Date

2026-08-11

## Context

在 980×680 的最小支持窗口中，连接、协议、命令和扩展配置页必须依赖纵向滚动才能访问，
而实时观测、终端和发送区长期占据首屏下半部。UI-1.126 已消除 tab/route 重叠，但没有解决
配置工作区首屏过矮的问题。用户需要一个明确、可逆且不影响设备工作的配置视图。

## Decision

在既有 `workspaceRouteStrip` 放置 `QToolButton#workspaceFocusButton`。它只改变当前 window
的 presentation mode：专注时将 `liveObservationBand`、`terminalSurface`、`sendControlBand`
的 `maximumHeight` 一次性动画到 0，并放开 `workspaceTabs` 的高度上限；总览时反向动画并恢复
三块 surface。模式默认关闭且不写入偏好。

`workspace_focus_transition.py` 负责状态、静态落点和 `QParallelAnimationGroup` 生命周期；
`workspace.py` 只负责组装和接线；`lifecycle.py` 在主题/低动效/暂停/隐藏/最小化/关闭边界调用
stop。按钮是键盘可达的，并使用 base/variant semantic token。

## Alternatives Considered

### 永久压缩或隐藏终端/发送区

拒绝：会损害总览模式的串口调试主路径，也会使用户无法同时观察和发送。

### resize handler 动态重排所有 surface

拒绝：引入持续尺寸状态和更大的布局耦合；专注模式是用户明确触发、可逆且更容易验证的
presentation 边界。

### 只滚动到配置页顶部

拒绝：仍保留过矮 viewport，且不提供持续配置空间；专注模式让四个设置页共享同一清晰入口。

## Consequences

- 980×680 下配置页可获得约 473px viewport，连接/UART 参数能在一个视图中完成主要配置。
- 总览模式完整保留实时观测、终端、发送和数据状态；切换不停止任何 backend/application worker。
- 低动效、暂停、窗口隐藏/最小化/关闭无残留动画；模式不跨重启持久化。
- 新增一个小型 presentation transition 模块和 Qt Core animation export，但不增加第三方依赖、
  QTimer、业务状态或 OTA/AES/RTT/J-Link 耦合。

## Verification

真实组合根覆盖动态中间帧、反向恢复、低动效静态、hide/show、三主题 × 980×680/1180×780 ×
四个 tab：exact-white=0、当前页 horizontal maximum=0、route/tab overlap=false。scripts/check.ps1、
compileall 和 Ruff 通过。架构师线程 `019fed62-45b0-7dc0-9a44-736aad31aff3` 在限定窗口内超时，
未计为独立通过；父代理完成 correctness/readability/architecture/security/performance 和简化审查。
未修改嵌入式 C/C++；public vendor applicability=N/A；真实 EXE 启动、硬件、读屏和正式签名验收仍未运行。
