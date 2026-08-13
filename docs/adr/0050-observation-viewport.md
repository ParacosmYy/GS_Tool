# ADR 0050：统一文本观察视口装饰边界

日期：2026-08-10

状态：accepted

## 背景

Terminal 已有上下观测标尺和扫描信号，而 Component/Dataset 预览仍是无装饰的普通
`QPlainTextEdit`。三类区域都需要保持原生文本编辑器的可复制、滚动、只读、placeholder
和无障碍契约，同时避免为每个区域各自创建绘制逻辑和动画时钟。

## 决策

- 新增 `presentation/observation_viewport.py:ObservationViewport`，以 `QPlainTextEdit`
  为基类，统一绘制上下标尺、扫描信号和有限 scope 节点。
- `TerminalViewport` 继续保留原有公开类型，作为 `scope="terminal"` 的兼容子类；
  Component/Dataset 使用同一 renderer 的 `component`/`dataset` scope。
- renderer 只读取自身原生文本控件、`ThemeSpec` 和 lifecycle 传入的 `(phase, animated)`；
  不保存 raw frame、解析结果、Dataset sample，不创建本地 timer，不改变 scroll bar、
  placeholder、AccessibleName 或 controller 更新链路。
- `lifecycle.py` 将三个视口接入既有共享 `MotionController` fan-out，统一处理低动效、暂停、
  隐藏、最小化和关闭时的静态回退。

## 被否决的方案

- 为 Component 和 Dataset 各复制一份 QPainter 视口：会让边界、主题修复和生命周期逐渐分叉。
- 将 preview 文本或解析状态搬进新的 presentation controller：会复制既有数据事实和更新责任。
- 为每个视口增加独立 QTimer：会破坏共享低频时钟和可见性生命周期约束。

## 后果与验证

共享 renderer 让三个观察区保持同一视觉语言，同时通过 scope accent 保留区域辨识度；
Terminal 的类型兼容性和 Component/Dataset 的原生文本 API 得以保留。UI-1.63 已完成静态门禁、
offscreen 文本/动画/生命周期验证、三主题近白像素断言，以及 980/1180 响应式检查；离屏环境的
PySide6 fonts directory 警告仅影响该环境的中文字形，不改变 Windows 系统字体结论。真实设备、
原生 HIDPI、签名和硬件验收仍未运行。
