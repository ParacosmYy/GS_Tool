# ARCH-116 / UI-1.189 扩展工具站响应式能力卡交接

日期：2026-08-12  
范围：`responsive_capability_grid.py`、`embedded_extension_panel.py` 的卡片网格与滚动/焦点边界。

## 结果

扩展工具站现在按内容宽度自适应卡片列数：980px 窄屏两列；1180/1240px 宽屏 OTA 传输三卡同排；
OTA 安全和 RTT/J-Link 双卡组保持两列。helper 只重排已有 card QWidget，不接管 capability state、
selection/detail、accessibility、Tab 顺序、native QScrollArea 或共享 120Hz 动效。

## 当前验证状态

`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`CAPABILITY_GRID_MATRIX_PASS=pass`（627 checks，0 failures）；`EXTENSION_LIFECYCLE_MATRIX_PASS=pass`
（662 checks，0 failures）；`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。

Qt offscreen 已知 PySide6 font-directory warning 只影响截图文字字形，不影响几何/焦点/滚动断言；真实
Windows 字体部署仍未验收。视觉截图确认窄屏两列和宽屏 OTA 三列，等权 stretch 的同排卡片允许 1px 取整差。

## 架构、审查与简化记录

架构师 `019ff4c3-7140-7ac1-b7ec-de6e736da7b1` 批准 helper 的无状态 owner 边界；架构师
`019ff4c8-5889-7ed3-b938-4c90949ab328` 批准 margins/spacing 列数契约、非法宽度和 1 列保底；要求均已落地。
此前架构审查 `019ff4c0-d586-7f43-9400-8d98740c7592`、`019ff4c3-7140-7ac1-b7ec-de6e736da7b1` 前置等待记录中，
超时不计为通过；本轮独立 reviewer `019ff4cc-806a-7380-bb99-680d604241ca` 超时关闭，未形成外部 findings。
父代理完成六轴 review 与行为保持 simplification assessment。

本轮没有嵌入式 C/C++、MCU、RTOS、ISR/DMA、驱动、OTA 固件或安全升级实现；public-vendor-source applicability
为 N/A，不作 MISRA/ISO 26262/认证合规声明。

## 交付产物

使用 `local-arch-116` 构建 onefile，并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；
canonical、root、root-latest 均为 `48,030,866` bytes，SHA-256 为
`334406DFEA71D5E4B25FB67088DD3B907250EE8529827CE4AB644FB6FEF780DA`；archive listing SHA-256 为
`ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过；
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
