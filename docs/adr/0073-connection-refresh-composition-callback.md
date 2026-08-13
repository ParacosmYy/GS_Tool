# ADR 0073：连接控件刷新回调归还组合根

日期：2026-08-10

状态：accepted（ARCH-6n）

## 背景

连接控件的 enable、hint 和 busy projection 由 `controllers/connection.py` 负责，但多个 owner controller 通过
`MainWindow._update_connection_controls()` 的单调用点 facade 触发它。这个 facade 不保存状态，也不提供 Qt 生命周期能力，
却让顶层窗口继续暴露一个与业务 controller 重复的转发边界。

## 决策

由唯一组合根 `controllers/bootstrap.py` 在窗口控件完成初始化期间绑定命名回调：

```python
window._refresh_connection_controls = partial(update_connection_controls, window)
```

各 controller 继续接收现有 window presentation owner，只把调用目标改为该组合根绑定的 callback；`MainWindow` 删除
`_update_connection_controls` 以及对应 domain/controller import。`connection.py` 仍是 affordance policy owner，未把规则复制到
bootstrap 或其它 controller。

## 取舍

- 采用单一、显式、可检索的 callback 名称，减少 facade，不引入无限制 callback registry、mixin 或 `__getattr__`。
- 不把本轮扩大成完整 WindowPort/feature DTO 迁移；剩余 controller 对 window widget 的依赖仍按 ARCH-6 后续切片逐族收窄。
- 不改变 session state、replay gate、TCP Server readiness、BLE busy、首屏 hydration、Qt signal 参数或关闭生命周期。

## 证据

- 静态门、`compileall`、ruff 通过；`MainWindow` 不再导入 `SessionState` 或 `update_connection_controls`。
- Qt offscreen 真实组合根向量通过：callback 可调用、旧 facade 不存在、窗口未显示即可初始化并关闭。
- 六个只读角色均被调用但在窗口内超时后关闭，未将超时视为通过；独立复核同样超时。父代理完成 correctness、readability/simplification、architecture、security、performance 五轴审查。
- 本轮无嵌入式 C/C++/MCU/固件改动，厂商公开资料与嵌入式 workflow 适用性为 N/A。
