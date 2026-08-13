# ADR-0172：总览态 workspace 高度预算与首帧收敛

## 状态

已接受（ARCH-121 / UI-1.194，2026-08-12）

## 背景

总览态同时显示连接配置、实时观测、终端和发送区。固定提高 workspace shell 的最小高度会在 980×720 的可用 root 高度内
挤压 observation/terminal/send；而 focus→overview、快速 resize 的 layout settle 还可能让旧高度短暂残留，造成用户看到组件挤在一起。

## 决策

1. `workspace.py::_ResponsiveWorkspaceShell` 是总览态 workspace 的唯一高度 owner；它按 root layout 当前 sibling 的
   `minimumHeight()/minimumSizeHint()`、margins 和 spacing 计算安全 overview floor。
2. `minimumHeight` 是 floor，不是实际高度上限。980×720 的 floor 约 168px；宽屏 floor 封顶 220px，剩余空间允许 shell 自然扩张。
3. 连接、协议、命令和扩展页面继续由既有 `QScrollArea` 承担内容 overflow；不新增 nested scroll 或 page height owner。
4. focus snapshot restore 完成后调用 shell 的兼容同步入口，重算 overview floor；focus 模式不触发 overview 同步。
5. resize/首显使用最多两轮 coalesced `QTimer.singleShot(0, ...)` settle，并用 pending/round guard、`shiboken6.isValid` 防止
   无界递归、重复排队或销毁对象回调。该 callback 不是常驻 timer，也不参与业务或 120Hz 动效。

## 被否决的替代方案

- 固定 190px floor：980×720 的 root sibling 预算不足，会出现约 16px overlap。
- 调大 root spacing/挤压终端：把视觉问题转成更小的终端 viewport，并改变既有垂直节奏。
- 修改 focus snapshot 字段：扩大生命周期 diff，容易把 overview floor 泄漏到 focus 恢复。
- 新增 geometry animation/timer：布局收敛不需要第二帧源，会与唯一 MotionController 和现有 transition 互相影响。

## 架构、审查与简化记录

架构师 `019ff53b-61e5-7a10-8ad9-a1f153e5903a` 批准高度 owner；`019ff549-d471-71b3-90aa-d8ba7f08933e` 批准动态
预算；`019ff54e-0953-7c21-a93f-3f77d09ce5e4` 批准一次性 queued settle；`019ff550-f612-7fe1-b11e-bee17163d8e9` 批准
最多两轮 guard。`019ff556-9dd3-7ef0-84ee-e9ccc78015ed` 拒绝了会破坏 pending 合并保护的额外入口清理，方案保持不变。
独立 reviewer `019ff553-2d64-7330-8ca5-fa873de955f7` 等待超时关闭，未形成外部 findings；父代理完成 owner、行为保持、
生命周期、可访问性、性能、可维护性与 simplification assessment。本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

## 验证

`compileall`、Ruff、`scripts/check.ps1` 通过。三主题×正常/低动效×`980×720`/`1180×820`/`1240×900`、快速 resize、
focus↔overview、hide/show/close 共 `312` checks、`0` failures；冷启动和快速宽屏→980 采样确认 overlap=`0px`，queued settle
最多两轮且最终 pending/round 清零。真实 GUI/HIDPI、EXE startup、硬件、显示器 120fps 和签名验收未运行。

## 交付指纹

`local-arch-121` onefile 已生成并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`。canonical、root、root-latest 均为
`48,040,231` bytes，SHA-256 为 `0524910A444C68B5437E94A73481F590E2F4865072E33158C703D1AFB79B80EC`；archive listing
SHA-256 为 `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过。签名为 `NotSigned`，
`release_eligible=false`，`hardware_acceptance=not_run`。
