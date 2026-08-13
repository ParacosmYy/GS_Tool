# ADR-0113：响应式工作区 viewport 空间边界

## Status

Accepted

## Date

2026-08-11

## Context

SerialForge 的最小支持窗口是 980×680。`workspaceShell` 同时容纳工作区 `QTabWidget`、固定 31px
路线条，以及实时观测、终端和发送 surface。原先 tab 声明 220px 最小高度，但在最小窗口的根布局
中父级实际只分配约 144px；Qt 仍让 tab 保持 220px，导致 route strip 覆盖 tab 内容，用户看到的
连接/协议设置首屏被裁切。

## Decision

由 `presentation/controllers/workspace.py:build_workspace_tabs()` 取消 `QTabWidget` 的 220px
硬最小高度，保留其最大高度 350px。小窗口下 tab viewport 让位给 route strip，页面内容继续由
现有 `QScrollArea#settingsScroll` 纵向承载；常用尺寸下保留既有布局行为。

## Alternatives Considered

### 在 resizeEvent 中增加动态高度状态

拒绝：需要新的生命周期/尺寸状态和额外 owner，容易把纯 presentation 几何扩散到 MainWindow，
也不能解决现有 scroll page 已经具备的承载能力。

### 隐藏 route strip 或压缩 terminal/send surface

拒绝：route strip 是稳定的工作区导航 affordance；terminal/send 是主路径，压缩或隐藏会改变
调试站的信息层级。让现有 viewport 收缩是最小可回滚的修复。

## Consequences

- 980×680 下 tab 与 route strip 不再重叠，设置页仍可通过既有纵向滚动访问完整内容。
- 1180×780 的工作区高度、Tab index、焦点顺序、主题切换、shared motion 和业务状态保持不变。
- 不新增业务状态、timer、依赖、设备 I/O 或 OTA/AES/RTT/J-Link 耦合。
- 未来若需要更丰富的 compact layout，应在独立 presentation owner 中提出并单独验证，不能把
  resize 判断塞入连接/协议 controller。

## Verification

真实组合根 offscreen 审计覆盖 980×680/1180×780、star_trail/moonlit_ocean/sakura_night、四个
workspace tab：exact-white=0，当前页 horizontal maximum=0，tab/route overlap=false，route
shell overflow=false。`scripts/check.ps1`、compileall 和 Ruff 通过。架构师线程
`019fed57-c98f-7330-a61a-7a69ce726a67` 在限定窗口内超时，未计为独立通过；父代理完成 owner、
几何、可滚动性、可访问性、性能和简化审查。未修改嵌入式 C/C++；embedded applicability=N/A。
