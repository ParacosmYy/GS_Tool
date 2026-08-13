# ADR 0140：协议/遥测页密度边界

- 状态：Accepted for ARCH-89 / UI-1.162
- 日期：2026-08-12

## 背景

协议页的运行时 binding 已稳定，但协议细节一次铺开 7 列，Dataset 标题同时承载状态和两个
动作，历史回放也把速度与三个动作放进一条横向带。窄窗口下用户看到的是控件互相争抢宽度，
而不是按任务理解配置、分析和回放。

## 决策

继续让 `controllers/protocol.py:build_protocol_panel()` 作为协议工作区组合 owner，并在其内部
采用语义布局：

- `protocol_detail_layout` 改用 `QVBoxLayout`，分隔符、长度字节、字节序通过
  `build_labeled_field()` 形成独立 peer fields；scope/timing 文案独占后续行。
- Dataset 的配置状态行与加载/导出动作行分离。
- Replay 的速度选择行与选择/暂停/停止动作行分离。
- 增大协议页 root 和单个 surface 的垂直 spacing，保留既有外层 scroll。

`ProtocolPanelWidgets` 的 37 个字段、`ProtocolPanelCallbacks`、业务 controller、状态投影、
Tab order、accessibility 和唯一 MotionController 不变。本轮不拆新 builder，因为新增文件不会
形成独立状态边界，反而扩大跨模块 binding 迁移面。

## 结果与验证

父代理完成 binding、依赖方向、行为保持、可读性/简化和动效不变量 fresh-pass；ARCH-89 架构师
只读调用和独立 review 调用均在服务窗口内超时并关闭，未伪造外部 PASS。已运行 Ruff、compileall、
工程约束、37-field import contract、source-limit、theme token audit 和 provenance verify；未启动
GUI/EXE，因此真实窗口几何、三主题截图、显示器 FPS、硬件/HIL 与签名验收均未运行。

交付 onefile：canonical、root、root-latest 均为 `48,004,714` bytes，SHA-256
`1D08E351935E0CA828C5AB095CD0712042F3C5F1DE6F0BD291595D508A6D3A44`，source revision
`local-arch-89`，archive listing SHA-256 `A152062F579FF9CCA9D2421EFDC6FA5D2FA6918A0691BE46EEE359AE451A47FF`，
签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
