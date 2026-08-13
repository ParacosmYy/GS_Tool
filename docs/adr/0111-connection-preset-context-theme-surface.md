# ADR-0111：连接快速配置上下文的语义主题 surface

- 日期：2026-08-11
- 状态：accepted for local engineering build
- 范围：presentation-only UI-1.124

## 决策

`ConnectionPresetContextSurface` 继续只接收既有 `ConnectionPreset | None`。它将来源投影为
`none`、`builtin` 或 `custom`，并将是否有选择投影为 `empty`/`selected`。属性变化统一经过
`refresh_dynamic_property()`，由 Qt 重新应用当前主题。

空态使用 neutral surface/border，内置 preset 使用 info surface/border 与蓝色 accent，自定义
preset 使用 history surface/border 与紫色 accent。自绘 panel/border 与 base/variant QSS 共享同一
组 ThemeSpec 语义 token，避免系统 palette 或近白背景回退。

## 边界与取舍

- surface 不调用 apply/connect，不保存 preset，不读取 ViewModel，也不创建新的连接状态源。
- controller、preset catalog/DTO、hint 文案、tooltip、AccessibleDescription、焦点路径和 shared
  MotionController frame/stop 不变。
- 不增加 timer、线程、I/O、外部资源或 OTA/AES/RTT/J-Link 依赖；嵌入式 C/C++ applicability=N/A。
- 架构师线程 `019fed48-9164-72c3-91f3-7472ab0cf517` 在限定窗口内超时；父代理完成 owner、QSS、
  polish、accessibility、性能和行为保持审查。

## 证据

三主题空态/内置/自定义 offscreen vector 通过，中心采样像素均非白色；`scripts/check.ps1`、
compileall、Ruff、onefile package 和 provenance verify 通过。canonical artifact 为
`dist/release/0.1.0/core/onefile/app/SerialForge.exe`，47,941,349 bytes，SHA-256
`79A366634CF0B87C629416194AC8891F7BFE6BB08CB680F3DE050D9A99D3E3BC`。
