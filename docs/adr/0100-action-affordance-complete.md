# ADR-0100：动作、字段与对话框 affordance 完整化

日期：2026-08-11  
状态：accepted（UI-1.110～UI-1.112）

## 背景

选择器完成不可编辑与基础说明后，回放、连接、录制、发送、UART、批量编辑、自定义连接和确认对话框仍存在提示粒度不一致的问题。尤其是动态动作状态、0 秒超时、CRLF payload、保存/取消和 QAction 填入行为，不能只依赖按钮文字或一个跨场景通用提示。

## 决策

- affordance 继续由原 presentation owner 声明，不建立全局 registry、Qt 文案服务或跨层状态源。
- 动态状态由既有 controller projection 更新 tooltip 与 accessible description；静态控件由构建 owner 设置基础说明。
- 输入字段、格式选择、payload 修饰和实际发送动作必须分别描述；字段说明不得暗示自动发送、自动连接、设备已接收或 OTA 已执行。
- 对话框按钮和快捷命令 QAction 必须说明保存、取消、确认、填入的范围；保存只保存现有 DTO，取消不写入，填入不自动发送。
- 超时仍复用既有 bounded selector；0 的不设定/不启用语义保持在 presentation 文案，不改变 runtime contract。

## 边界与验证

本决策只改变 presentation tooltip/accessibility 文案和同步时序，不新增业务状态、timer、signal、线程、依赖、设备 I/O、公开 API 或业务 DTO。UI-1.110～UI-1.112 的真实组合根 offscreen vectors、三主题渲染、近白像素门禁、关闭生命周期、静态检查、compileall、ruff 和 provenance 均通过。独立复核代理超时不计为通过，父代理完成五轴审查与行为保持简化评估。

详细证据见 docs/handoffs/2026-08-11-ui-1-110-112-affordance-completion.md。
