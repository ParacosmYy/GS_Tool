# ADR 0075：MainWindow 收敛为 composition shell

日期：2026-08-10

状态：accepted（ARCH-6p）

## 背景

虽然 feature controller 已经拆分，`MainWindow` 仍保存大量 `_on_*` / `_update_*` 纯转发方法。这些方法只把
`window` 传给对应 owner controller，却让顶层类看起来像业务入口，增加维护成本并掩盖 callback 依赖。

## 决策

在 `controllers/bootstrap.py` 增加按 feature owner 分域的显式 wiring：

- `_bind_protocol_callbacks`
- `_bind_derived_callbacks`
- `_bind_replay_callbacks`
- `_bind_ble_callbacks`
- `_bind_terminal_callbacks`
- `_bind_lifecycle_callbacks`

每个绑定函数只使用 `partial(owner_function, window)`，由 `_bind_controller_callbacks()` 在首个 timer/signal 使用前调用。
`MainWindow` 删除全部业务转发，只保留构造和 Qt 生命周期覆写；已有 controller 调用名称和 signal wiring 不改写成隐式注册表。

## 取舍

- 组合根承担 wiring 是其职责；业务规则继续在 protocol/derived/replay/BLE/terminal/lifecycle owner 内，bootstrap 不读取 ViewModel 状态来做判断。
- 保留具名 callback 属性，方便现有 builder/controller 和 Qt signal 逐步迁移；不使用 `__getattr__`、mixin、无限制 dict、monkey-patch 或共享 God object。
- 本轮不引入完整 WindowPort/feature state DTO；那是后续 ARCH-6 的更大迁移，避免一次性重写破坏生命周期可追溯性。

## 证据

- `MainWindow` 源文件从大量 facade 收敛到构造 + 4 个 Qt lifecycle override，所有 Python 文件仍不超过 1000 行。
- static/compileall/ruff 通过；真实 Qt offscreen composition vector 确认 14 个代表性 callback 可调用、旧业务方法不再存在、窗口可关闭。
- 六个只读角色与独立复核均被调用但在窗口内超时后关闭，未把超时视为通过；父代理完成 correctness、readability/simplification、architecture、security、performance 五轴审查。
- 本轮无嵌入式 C/C++/MCU/固件改动，厂商公开资料与嵌入式 workflow 适用性为 N/A。
