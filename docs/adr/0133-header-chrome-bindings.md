# ADR 0133：全局 Header Chrome 绑定边界

- 日期：2026-08-11
- 状态：accepted
- 关联切片：ARCH-6y / UI-1.141

## 背景

顶部状态摘要、品牌装饰、动效开关和主题选择器由 workspace builder 创建，
但 lifecycle/composition 仍通过 `MainWindow` 动态字段读取。这样状态投影、共享
动效 fan-out 和键盘路径容易依赖隐式初始化顺序。

## 决策

新增 `presentation/chrome_bindings.py` 的 frozen/slots `HeaderChromeBindings`。
它只保存 header-owned Qt 引用：status cluster、context/source/state label、
status indicator、motion controls/checks、theme controls/combo/swatch、brand mark
和 signal field。`controllers/workspace.py` 在首次 `on_theme_changed()` 前唯一组装；
`lifecycle.py` 与 `composition.py` 通过 `header_chrome_bindings_for()` 消费。

偏好 DTO、MotionController、主题策略、app root、error bar、status footer、ViewModel、
业务 callbacks、timer 和动作策略不进入 bundle。builder 动态字段仅用于构建和 signal 接线，
不是新的跨 controller facade。

## 不变量与验证

保持主题选项/图标、低动效和暂停语义、120Hz PreciseTimer/8–9ms 目标、shared frame/stop、
关闭/隐藏静态回退、Tab/focus/accessibility、三主题和 980/1180 布局。初始化早期主题
回调在 bundle 尚未存在时安全返回；正式构建完成后由 builder 进行首次主题投影。

本轮只涉及 Python/Qt presentation，不涉及嵌入式 C/C++、MCU、SDK、RTOS 或硬件；没有
适用的厂商一手要求，也不声称 MISRA、ISO 26262、ASIL、ASPICE 或认证合规。验证使用
compileall、Ruff、`scripts/check.ps1` 和真实 offscreen 组合根向量，不创建测试资产、不
闪写/擦除/部署硬件。
