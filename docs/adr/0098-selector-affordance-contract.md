# ADR-0098：用户选择器 affordance 契约

- 状态：Accepted
- 日期：2026-08-11
- 范围：`presentation/controllers/connection_builder.py`、`presentation/controllers/terminal.py`、`presentation/command_batch_editor.py`

## 背景

组合根离屏审计发现，主窗口的功能选择器虽然已有 bounded itemData 和业务 signal，但部分控件缺少一致的提示与无障碍描述，且“不可手输”的意图只依赖 Qt 默认值。用户需要能直接理解每个选择器的作用、影响范围和空态行为。

## 决策

各 presentation owner 显式声明选择器 affordance：

- transport、终端显示/发送格式、发送历史、批量命令和批量编辑器格式/快捷命令显式 `setEditable(False)`；
- 每个用户选择器提供 tooltip 与 accessible description，说明选择结果是否只改变展示、是否需要显式执行，以及不会自动连接/发送的边界；
- UART 端口保持唯一的 `setEditable(True)` 选择器，允许枚举端口或输入 `COMx`，但同样提供完整 accessibility 文案；
- 不抽取跨 owner 的万能 helper，不复制 itemData、枚举、业务状态或 signal wiring；每个 builder/editor 继续拥有自己的控件契约。

该决策使用户可见 affordance 与模块 owner 同步，避免将 UI 说明渗透到 application/domain，同时保持现有主题 QSS、连接 gate、快捷键和 OTA/debug 边界不变。

## 验证与风险

真实组合根离屏检查 27 个 `QComboBox`，确认只有 UART 端口可编辑，全部有 accessible name/tooltip/accessible description；三主题、980×680/1180×780 渲染均无近白像素，关闭时取消 discovery 并等待线程池。静态门禁、compileall、ruff 与 provenance 校验通过。未运行可见 GUI/EXE startup、真实传输、硬件、OTA 或正式发行验收。

本轮仅修改 Python/Qt presentation，不适用 MCU vendor public source，不声明 MISRA/ISO/硬件合规。六角色前置评审两轮和最终独立复核均在限定窗口内超时并关闭，超时不视为通过；父代理完成五轴审查、边界复用检查和非破坏性验证。

