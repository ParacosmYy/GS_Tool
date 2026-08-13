# ARCH-121 / UI-1.194 总览态 workspace 高度预算交接

日期：2026-08-12  
范围：总览态 workspace shell 的纵向高度预算、focus restore 后同步和 resize 首帧 settle；不涉及业务或设备路径。

## 结果

`_ResponsiveWorkspaceShell` 根据 root sibling 最小高度预算动态设置 overview `minimumHeight`：980×720 floor 约 168px，
连接 viewport 约 97px；宽屏 floor 封顶 220px，实际高度可自然扩张。focus snapshot restore 后由同一 owner 重同步；首显/快速
resize 最多两轮 queued settle，避免连接页与实时观测栏短暂重叠。既有 QScrollArea、bindings/signals、唯一 120Hz MotionController、
主题、业务和 OTA/AES/RTT/J-Link 边界不变。

## 当前验证状态

`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`TERMINAL_OVERVIEW_THEME_LIFECYCLE=pass`（312 checks，0 failures）；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。

offscreen PySide6 font-directory warning 只影响环境提示；当前字体注册链路已验证中文正常显示。未新增或运行 unit test、mock、fixture、
harness 或 test-only 资产。独立 reviewer 超时关闭，未形成外部结论。

## 架构与审查

架构师 `019ff53b-61e5-7a10-8ad9-a1f153e5903a`、`019ff549-d471-71b3-90aa-d8ba7f08933e`、
`019ff54e-0953-7c21-a93f-3f77d09ce5e4`、`019ff550-f612-7fe1-b11e-bee17163d8e9` 正式批准；
`019ff556-9dd3-7ef0-84ee-e9ccc78015ed` 拒绝额外 pending 清理改动，已保留现有两轮合并保护。父代理完成六轴 review 与
behavior-preserving simplification assessment。本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

## 交付

源码与 Qt offscreen 验证已完成。使用 `local-arch-121` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,040,231` bytes，SHA-256 为
`0524910A444C68B5437E94A73481F590E2F4865072E33158C703D1AFB79B80EC`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名、正式发行资格与硬件验收
为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
