# ADR 0074：Transport 与 protocol framing callback 归还组合根

日期：2026-08-10

状态：accepted（ARCH-6o）

## 背景

`MainWindow` 仍有 `_on_transport_changed()` 与 `_on_protocol_framing_changed()` 两个无状态转发方法。它们只是把 Qt
signal 或首屏调用转交给 `connection_runtime.py` / `protocol_config.py`，没有持有窗口生命周期或渲染规则，继续保留会让
composition shell 暴露重复的业务入口。

## 决策

由 `controllers/bootstrap.py` 在其它控件调用前安装两个显式 callback：

```python
window._on_transport_changed = partial(on_transport_changed, window)
window._on_protocol_framing_changed = partial(on_protocol_framing_changed, window)
```

既有 builder、preset、replay、composition 和 protocol controller 调用目标保持同名，只删除 `MainWindow` 方法和两个 import。
各 owner 继续拥有 transport panel projection、协议 framing 草稿副作用和 Qt payload 边界。

## 取舍

- 采用可检索的命名 callback，保持本轮增量小且可回滚；不引入无限制 callback registry、mixin、`__getattr__` 或 WindowPort 全量重构。
- 不改变 `on_transport_changed()` 的 six-mode projection、panel transition、connection controls refresh 顺序或
  `on_protocol_framing_changed()` 的 editor/status/timing side effects。
- 其它 MainWindow facade 继续按 ARCH-6 逐族迁移，不在本切片批量改写 controller state access。

## 证据

- static/compileall/ruff 通过；真实 Qt offscreen 组合根确认两个 callback 可调用且类上旧 facade 不存在。
- 六个只读角色均已调用但窗口内超时后关闭；独立复核也超时，未将超时视为通过。父代理完成 correctness、readability/simplification、architecture、security、performance 五轴审查。
- 本轮无嵌入式 C/C++/MCU/固件改动，厂商公开资料与嵌入式 workflow 适用性为 N/A。
