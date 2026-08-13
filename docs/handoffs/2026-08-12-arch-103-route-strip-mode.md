# ARCH-103 / UI-1.176 交接：路线条模式主题层级

日期：2026-08-12

## 结果

复用既有 `workspaceShell[mode="focus|overview"]` 动态属性，为底部路线条增加明确的模式视觉：
专注设置使用 info→history 渐变与蓝色上沿，总览使用 neutral 上沿。路线条仍为 31px 固定高度，
按钮、Tab、滚动提示、键盘路径和 accessibility 文案完全复用原有 owner；没有新增状态源、timer、
线程、animation owner、transport/session/OTA/debug coupling。

## 证据

- `uv run ruff check ...`：pass。
- `uv run python -m compileall -q src`：pass。
- `scripts/check.ps1` / theme audit：将在最终交付前复跑。
- 三主题 focus/overview 实跑：`mode=focus|overview`、按钮文案和滚动提示状态均正确。
- 980/1240、四 workspace：`horizontal_max=[0, 0, 0, 0]`，sibling geometry 无重叠。
- 共享时钟：0.5 秒 `frames=60`、均值 `8.41ms`、有效约 `118.8Hz`；只作为 scheduler target 证据。
- 主题截图：`C:\Users\Gs\AppData\Local\Temp\serialforge-arch104-route-star_trail-focus.png`、
  `serialforge-arch104-route-star_trail-overview.png`、
  `serialforge-arch104-route-moonlit_ocean-focus.png`、
  `serialforge-arch104-route-sakura_night-overview.png`。

## 架构与审查记录

- 架构师：`019ff3ac-0f16-7633-8304-1cb3a1cecb54`，等待窗口超时关闭，未形成外部结论。
- 独立 reviewer：`019ff3b1-8e10-7b63-94c3-af41db79e1a5`，等待窗口超时关闭，未形成外部结论。
- 父代理复核：selector 优先级、三主题 token、固定高度、焦点/无障碍和行为保持；简化判断为复用
  已有 `mode` 属性，不新增 controller 或状态类。
- embedded C/C++ public-vendor-source applicability：N/A；没有固件、MCU、BSP/HAL/RTOS、OTA 实现或
  C/C++ 修改，不作 MISRA、ISO 26262、ASIL、ASPICE 或认证声明。

## 交付与未运行项

本轮 onefile 已使用 `local-arch-103` 打包，并覆盖 canonical、根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`；三者均为 `48,016,499` bytes，SHA-256 为
`B51F8C0D0402CF692F41DC16CCAD70E183F2C764850DCAA08890CF7709292CEE`，archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。EXE 启动、真实 Windows
可见窗口/高刷新显示器/HIDPI、高负载、硬件连接、OTA/RTT 实连、签名和正式硬件验收仍未运行或未授权。
