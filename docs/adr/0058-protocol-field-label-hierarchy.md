# ADR 0058：协议配置字段标签使用主题次级层级

日期：2026-08-10

状态：accepted

## 背景

协议/遥测配置页的普通字段标签仍使用未带语义属性的原生 `QLabel`。在深色二次元主题中，系统 palette 或默认 label
对比度可能把字段标签渲染成过亮甚至白色，破坏 section 标题、字段、状态和 hint 之间的层级。连接页已经有局部
`_field_label()` 约束，协议页需要同样的稳定边界。

## 决策

在 `presentation/controllers/protocol.py` 内增加局部 `_field_label(text)` helper。它只创建 `QLabel` 并设置
`role="muted"`，让既有默认 stylesheet 与三套 theme override 负责颜色和对比度。以下 8 个普通字段使用该 helper：

- 预设、Framing、校验、最大帧(B)
- Delimiter Hex、长度字节、字节序、过滤

协议 section 标题继续使用 `role="section"`；状态、hint 和动态说明继续由原 owner 保留，不进入普通字段批量替换。
不新增状态、timer、依赖、DTO 字段、callback 或全局 label factory。

## 未采用方案

- 不在全局 stylesheet 中按文本匹配标签：文本不是稳定的组件边界，也会误伤 section/status/hint。
- 不新增通用跨页面 label 服务：当前只存在两个局部表单边界，跨页面抽象会扩大依赖面。
- 不用固定颜色或系统 palette：这会重新引入主题切换时的白色 fallback。

## 验证

- `scripts/check.ps1`：pass；148 个 Python 文件均不超过 1000 行，3 套主题、22 个语义 token、19 个 selector，Ruff/compileall pass。
- `python -m compileall -q src`：pass。
- 三主题短时 Qt offscreen 协议页 vector：8 个 `role="muted"` 字段、5 个 `role="section"` 标题、5 个状态 owner 均通过。
- 独立质量复核代理 `019feb8b-9a95-73f2-9966-87d7c0fc2a7f` 在 30 秒窗口内超时并关闭；未将超时记录为通过，父代理完成 correctness/readability/
  architecture/security/performance 五轴审查与行为保持检查。
- 未启动完整 GUI/EXE、未接入真实 UART/TCP/BLE/RTT/J-Link/OTA、未做 HIDPI/读屏或硬件验收；嵌入式 C/C++ 适用性：N/A。
