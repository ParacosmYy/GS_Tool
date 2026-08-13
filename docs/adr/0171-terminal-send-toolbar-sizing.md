# ADR-0171：实时观测与发送栏 intrinsic sizing

## 状态

已接受（ARCH-120 / UI-1.193，2026-08-12）

## 背景

总览态的实时观测栏和发送栏都使用横向布局。Qt 的等权伸缩会把清空/记录、发送模式、快捷命令等动作控件
拉宽，导致 980px 下按钮视觉权重失真；这些动作本身有稳定的 intrinsic size，而输入与摘要更适合吸收剩余空间。

## 决策

1. `_ResponsiveObservationBand` 只调整已有实时观测行的 spacing 和 widget policy；按钮固定为 Qt `Fixed`，
   状态标签保持 bounded preferred；
2. `_configure_send_layout_sizing()` 只调整发送栏已有 QWidget policy：mode/send/newline/quick/save 为 `Fixed`，
   input/context 继续伸缩，state 保留原有 100–180px bounded contract；
3. 不用 `setFixedWidth(sizeHint())`，让 Qt 根据当前字体、DPI 和翻译动态计算 intrinsic width；
4. 不重建控件，不改变 bindings、signals、callbacks、业务状态、滚动、动画、MotionController 或传输边界。

## 被否决的替代方案

- 直接写死 980px 断点：无法适应 DPI/字体/翻译，且把布局策略泄漏到窗口尺寸；
- `setFixedWidth(sizeHint())`：构建期冻结宽度，后续字体/DPI变化可能截断；
- 将状态/快捷操作复制成第二套窄屏控件：会复制 identity、accessibility 和 signal 生命周期；
- 新增 timer/动画实现“平滑重排”：布局策略不需要额外帧源。

## 架构、审查与简化记录

架构裁决 `019ff51d-9728-75b2-9f96-c144beeec9b1` 正式 `APPROVE`；Terra 微裁决
`019ff523-c27e-7bd2-8c2c-c46c4b99527e` 正式 `APPROVE`，明确禁止冻结像素宽度。较宽独立审查线程
超时未形成结论；独立 reviewer `019ff526-3e94-7761-bccd-96153086bfcb` 等待超时关闭，未形成外部 findings；
父代理完成 owner、行为保持、生命周期、可访问性、性能、security/readability 与 simplification assessment。
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

## 验证

三主题×总览态多尺寸与往返 resize、低动效、hide/show/close 共 36 checks、0 failures；check、compileall、
Ruff 通过。真实 GUI/HIDPI、EXE startup、硬件、显示器 120fps 和签名验收未运行。

## 交付指纹

`local-arch-120` onefile 已生成并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`。canonical、
root、root-latest 均为 `48,038,525` bytes，SHA-256 为
`05A72BF2599736C65BA4B43CFB525F72FED4EB2C2CA4E47672BAC46B2E07165B`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过。签名为
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
