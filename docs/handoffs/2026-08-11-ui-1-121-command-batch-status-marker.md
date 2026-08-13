# UI-1.121 批量命令状态 marker

日期：2026-08-11  
父代理：Codex；本地 checkout 唯一写入者  
范围：presentation-only 命令管理状态可视化

## 变更

- `CommandBatchSurfaceLabel` 在原生文字绘制后增加 state marker：ready/completed 使用 success，
  running/stopped 使用 warning，failed 使用 error，empty 使用 neutral border。
- running marker 的呼吸环复用已有 lifecycle shared frame；步骤 rail、状态文字、snapshot、结果表、
  batch gate、NoFocus、鼠标透明和 reduced-motion/static fallback 保持不变。
- `theme_stylesheet_base.py` 将 `commandBatchStatus` 左 padding 调整为 `28px` 并提升字重；
  `theme_variant_shell.py` 的三主题 semantic status selector 保持原有 owner，不新增业务判断。

## 验证

- `scripts/check.ps1`：pass；source line limit `157 files <= 1000`；theme token audit pass。
- compileall：pass；Ruff：pass；provenance verify：pass。
- `UI121_COMMAND_VECTOR_PASS`：三主题 `star_trail / moonlit_ocean / sakura_night`，生产字体
  `Microsoft YaHei UI`，surface `720x120`，label `688x88`，projection、NoFocus、鼠标透明和
  非白色主题背景断言通过；截图已人工查看。
- 未运行真实 GUI EXE 启动、HIDPI、读屏、真实串口/网络/BLE/RTT/J-Link、OTA 或硬件验收。

## 架构与 assurance

架构师线程 `019fed34-6372-7050-ab70-437f34378271` 在限定窗口内超时，未计为独立通过；父代理
完成 owner、token、Qt 绘制、accessibility、简化和生命周期审查。未修改嵌入式 C/C++、固件、
BSP/HAL、RTOS、驱动、协议实现、OTA 或 RTT/J-Link 后端；embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.121`；47,941,026 bytes；SHA-256
`7A9DC10ED43B2B9876B2FCF03F422A70E14765901E616206AF9041940DF2D20F`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。
