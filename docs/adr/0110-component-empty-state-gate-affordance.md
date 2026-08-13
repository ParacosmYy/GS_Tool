# ADR-0110：Component 空态 waiting/blocked gate affordance

日期：2026-08-11  
状态：Accepted  
范围：SerialForge presentation / Component telemetry empty state

## 背景

Component 空态在来源不支持时已经会隐藏/禁用 Profile/Codec CTA，并将 glyph 降级，但卡片边界
和眉题仍写成等待态。用户无法快速区分“可用但还没有帧”和“当前来源不支持解析”。

## 决策

- 不新增业务状态源；`ComponentEmptyStateSurface.set_action_enabled()` 只把现有 gate 投影成
  `waiting` 或 `blocked` presentation property，并同步 `COMPONENT / WAITING` /
  `COMPONENT / BLOCKED` 眉题。
- 使用 `refresh_dynamic_property()` 完成 Qt property 的幂等 re-polish/update；base 与 variant
  stylesheet 对称提供 blocked 的 neutral surface/warning border。
- 保留 `setText()/text()` 完整文案、无参数 `load_requested`、CTA enabled/visible、glyph available、
  shared MotionController frame/stop、NoFocus、鼠标透明和无障碍契约。
- 不改变 `protocol_config.py` source gate、`derived_data.py` rows/table 互斥、codec/profile、
  transport、OTA/AES/RTT/J-Link 或设备 I/O。

## 验证

静态门禁、compileall、Ruff、provenance verify 和三主题 waiting/blocked offscreen vector 通过；
生产字体为 `Microsoft YaHei UI`，CTA、眉题、state property、NoFocus、鼠标透明和可访问描述已核对。

架构师线程 `019fed42-2796-7fb2-9fe8-0e09a22026c8` 在限定窗口内超时，未计为独立通过；父代理
完成 owner、token、Qt property、可访问性、简化和生命周期审查。未修改嵌入式 C/C++；embedded
applicability=N/A。
