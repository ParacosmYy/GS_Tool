# SerialForge UI-1.63 交接：ObservationViewport

日期：2026-08-10  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 交付结果

本轮把 Component/Dataset 的纯文本预览接入共享 `ObservationViewport`。它以原生
`QPlainTextEdit` 为基类，增加上下观测标尺、扫描信号和有限 scope 节点；Terminal 通过
兼容子类继续使用 `TerminalViewport` 类型。scope 只改变主题 accent 和节点数量，不改变
任何协议、解析、Dataset、滚动或文本事实。

保留的既有契约包括：

- Component/Dataset controller 的原生文本更新、`maximumBlockCount`、placeholder、只读和无障碍描述；
- TerminalViewport 的公开类型和 terminal scope；
- lifecycle 共享 `MotionController` 的单一 frame/stop fan-out；
- reduced-motion、暂停、隐藏、最小化和关闭时的静态回退；
- 三主题及 980/1180 响应式边界。

## 文件与架构边界

- `src/serialforge/presentation/observation_viewport.py`：共享 QPlainTextEdit 装饰 renderer。
- `src/serialforge/presentation/terminal_surface.py`：TerminalViewport 兼容层。
- `src/serialforge/presentation/controllers/protocol.py`：Component/Dataset 预览装配。
- `src/serialforge/presentation/controllers/lifecycle.py`：共享动画生命周期接入。
- `docs/adr/0050-observation-viewport.md`：决策、否决方案和后果。

架构角色 `019feb3d-0086-7663-97c5-d59fab58391e` 在源代码修改前调用，等待超时后关闭；
独立复核角色 `019feb41-569c-7ec2-a5da-07155e5ab811` 在实现后调用，等待超时后关闭；
为绘制路径简化再次调用架构角色 `019feb43-6a6d-7530-8629-9fb9a284aaef`，同样等待超时后关闭。
父代理完成五维复核：正确性、可读性/简洁性、架构边界、安全性、性能均无必须阻断项；
简化项为复用一次已解析的 `ThemeSpec.surface_input`，删除 scope 节点循环中的重复主题解析。

## 验证证据

```text
check.ps1                         PASS  143 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
UI163_OBSERVATION_VIEWPORT_SMOKE PASS  motion_surface_count=34; component_motion_delta=84; native text/scope/lifecycle pass
theme pixel audit                 PASS  Component/Dataset all three themes near_white=0
responsive audit                  PASS  protocol page horizontal scroll maximum=0 at 980x680 and 1180x780
provenance.py verify              PASS  local-ui-1.63 final onefile manifest
```

离屏 Qt 报告 PySide6 fonts directory 不存在；这只影响离屏环境中文字形，不作为 Windows 字体结论。
没有运行真实设备、持续 GUI、原生 HIDPI/读屏、硬件、签名或正式发行验收；嵌入式 C/C++ 适用性为 N/A。

## 最终包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.63`
- size：`47,874,535` bytes
- SHA-256：`55D2682DA194658480DDCB73996D832C12568704A923696967013550BC1CF1BF`
- archive listing SHA-256：`1237F26EAB85E74ACEA75E347C29155E49813297F741AADA75517D8431EAF387`
- signature：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
