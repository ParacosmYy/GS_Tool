# UI-1.123 Component 空态 gate affordance

日期：2026-08-11  
父代理：Codex；本地 checkout 唯一写入者  
范围：presentation-only Component 空态状态反馈

## 变更

- `ComponentEmptyStateSurface` 初始化为 `state="waiting"`；`set_action_enabled(True/False)`
  通过 `refresh_dynamic_property()` 投影 `waiting/blocked`，并同步 `COMPONENT / WAITING` /
  `COMPONENT / BLOCKED` 眉题。
- base/variant shell QSS 增加 blocked neutral surface/warning border；waiting 的 blue surface、
  CTA、glyph 和共享帧动效保持不变。
- 没有改动 controller、`setText()/text()`、`load_requested`、source gate、rows/table 互斥、焦点、
  无障碍或 OTA/debug 边界。

## 验证

- `scripts/check.ps1`：pass；source line limit `157 files <= 1000`；theme token audit pass。
- compileall：pass；Ruff：pass；provenance verify：pass。
- `UI123_COMPONENT_VECTOR_PASS`：三主题 `star_trail / moonlit_ocean / sakura_night`、waiting/
  blocked 两态，生产字体 `Microsoft YaHei UI`；state、眉题、CTA visible/enabled、NoFocus、鼠标透明、
  AccessibleDescription 和截图均通过。
- 首轮向量脚本误用了不存在的公开 `action_button` 成员；这是验证脚本错误，已修正为现有 `_action`
  presentation owner 后重新通过，源码未为测试暴露新 API。
- 未运行真实 GUI EXE 启动、HIDPI、读屏、真实串口/网络/BLE/RTT/J-Link、OTA 或硬件验收。

## 架构与 assurance

架构师线程 `019fed42-2796-7fb2-9fe8-0e09a22026c8` 在限定窗口内超时，未计为独立通过；父代理
完成 owner、token、Qt property polish、accessibility、简化和生命周期审查。未修改嵌入式 C/C++、
固件、BSP/HAL、RTOS、驱动、协议实现、OTA 或 RTT/J-Link 后端；embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.123`；47,939,601 bytes；SHA-256
`BC3D81E7714111A63B18DC9679B34A6F7D874830BEF4537F56116F07ED5B36D1`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。
