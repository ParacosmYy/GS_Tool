# ADR 0049：链路传输模式几何 glyph

日期：2026-08-10  
状态：accepted  
范围：`presentation/transport_mode_surface.py`、`controllers/connection_builder.py`、`controllers/connection_runtime.py`

## 背景

链路页的传输方式已经由有界 `QComboBox` 提供选择，header context 和 connection hint 也提供了文字上下文；但六类
UART/TCP/UDP/BLE/RTT 在视觉上都只是同一种下拉控件，切换模式时缺少即时的几何识别信号。直接增加独立卡片会重复
preset summary、SessionState 和 hint 文案，并挤压 980px 响应式布局。

## 决策

- 新增 `TransportModeSurface`，固定 42×26，在“传输”标签旁绘制六类资源无关 glyph。
- `set_mode()` 只接受 combo 投影出的有界字符串；模式选择、键盘顺序、AccessibleName/Description 和所有连接动作仍归
  原生 `_transport_combo` 与既有 controller 所有。
- glyph 只消费 `ThemeSpec` 和共享 `MotionController` frame/stop；不创建本地 timer，不读取 ViewModel/SessionState，
  不自动切换面板、不修改配置、不连接设备。
- `connection_runtime.py` 在既有 `on_transport_changed()` 中同步 mode，`lifecycle.py` 将 surface 纳入现有统一 fan-out；
  不新增业务状态或组合根依赖。

## 被拒绝的替代

- 新建 transport context state/controller：已有 combo、header context、connection hint 和 `ConnectionPresetContextSurface`
  已拥有事实/文字契约，新增状态会产生重复来源。
- 用六个可点击按钮替换 combo：会改变键盘/无障碍/连接面板选择 owner，并扩大交互回归面。
- 为每个 glyph 创建动画 timer：违反共享 MotionController 和隐藏/暂停/关闭静态回退约束。

## 后果与验证

链路页在不增加用户配置步骤的情况下获得模式识别和共享动态信号；原有连接语义、配置和文案不变。验证包括静态门禁、
六模式面板显隐、共享 frame/stop、动效像素差、三主题 near-white=0、980/1180 无横向滚动和 onefile/provenance。
真实传输、HIDPI、读屏和正式发行验收仍不在本 ADR 范围内。
