# ADR-0101：协议与派生数据动作 affordance

日期：2026-08-11  
状态：accepted（UI-1.113）

## 背景

协议/组件/Dataset 页面已有加载与导出动作，但在禁用来源下四个按钮共享同一条“派生不可用”提示，用户无法提前知道点击动作本来会做什么，也无法区分 Profile、组件 CSV、Dataset 配置和 Dataset CSV 的副作用。

## 决策

- 四个动作继续由 protocol.py 创建，由 protocol_config.py 在既有来源 gate 中分别投影 enabled、tooltip 和 accessible description。
- 四条基础文案由 presentation/protocol_action_hints.py 的不可变 contract 单一提供；两个 owner 不重复维护文案，也不把它扩展成全局动作 registry。
- 启用态描述实际动作与不影响范围；禁用态在动作描述后追加现有来源原因。
- 不引入全局 affordance registry、第二套派生状态、跨层 Qt 文案服务、动作确认框或新的业务 DTO。
- 既有回调、焦点顺序、Tab 顺序、主题 QSS、关闭生命周期、原始终端/记录、发送与 OTA/debug 边界保持不变。

## 验证与限制

三套主题和 enabled/disabled 两态的四动作 offscreen vector、静态门禁、compileall、ruff 与 provenance 通过。独立子代理复核因运行时线程上限未启动，父代理审查已记录但不能替代独立复核。

证据见 docs/handoffs/2026-08-11-ui-1-113-derived-action-affordance.md。
