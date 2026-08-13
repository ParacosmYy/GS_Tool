# UI-1.122 分析状态 marker

日期：2026-08-11  
父代理：Codex；本地 checkout 唯一写入者  
范围：presentation-only 协议派生状态可视化

## 变更

- `AnalysisStatusLabel` 为 protocol/component/dataset/curve/replay 共用 status label 增加左侧
  semantic marker；颜色只复用已有 `state/source` 语义映射。
- active/waiting/draft 在 shared frame 可用时增加低对比度 pulse；原生文字、AccessibleDescription、
  底部五节点 rail、NoFocus、鼠标透明和 reduced-motion/static fallback 保持不变。
- `theme_stylesheet_base.py` 将共享 status selector 左 padding 调整为 `22px` 并提升字重；variant
  QSS 继续只提供主题 token，不新增业务状态判断。

## 验证

- `scripts/check.ps1`：pass；source line limit `157 files <= 1000`；theme token audit pass。
- compileall：pass；Ruff：pass；provenance verify：pass。
- `UI122_ANALYSIS_VECTOR_PASS`：真实组合根 `980x680/1180x780`、三主题、协议/遥测页，horizontal
  scroll range 均为 `0`；五类 status width `[520, 440, 420, 420, 520]` 均不低于 minimum size
  hint `[200, 200, 200, 200, 76]`；孤立 active/waiting/history/error 图已人工查看。
- 生产字体为 `Microsoft YaHei UI`；未运行真实 GUI EXE 启动、HIDPI、读屏、真实串口/网络/BLE/
  RTT/J-Link、OTA 或硬件验收。

## 架构与 assurance

架构师线程 `019fed3c-101d-7873-bef8-bde944acc6b8` 在限定窗口内超时，未计为独立通过；父代理
完成 owner、token、Qt 绘制、accessibility、简化和生命周期审查。未修改嵌入式 C/C++、固件、
BSP/HAL、RTOS、驱动、协议实现、OTA 或 RTT/J-Link 后端；embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.122`；47,941,295 bytes；SHA-256
`E881FA27DD8B0564C195A95A2B94A1CF521772438AA18A6EEAD1A6BF0957FB55`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。
