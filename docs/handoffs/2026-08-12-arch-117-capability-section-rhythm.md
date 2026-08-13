# ARCH-117 / UI-1.190 扩展工具站能力分区交接

日期：2026-08-12  
范围：`presentation/embedded_extension_panel.py` 的能力组分区外框与垂直节奏。

## 结果

每个能力组现在由一个 `QFrame#extensionCapabilitySection` 统一承载标题、说明和既有响应式卡片网格，
上下 10px、内部 spacing 7px、水平 margin 0。视觉层级被分成三个独立分区，同时保留 1180px OTA 三卡
三列和 980px 两列。

## 当前验证状态

`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`EXTENSION_SECTION_MATRIX_PASS=pass`（318 checks，0 failures）；`EXTENSION_SECTION_LIFECYCLE_PASS=pass`
（38 checks，0 failures）；`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；
`GUI/EXE-startup=not-run`；`hardware=not_run`；`release=ineligible`。

Qt offscreen 已知 PySide6 font-directory warning 只影响截图字形，不影响几何/焦点/滚动断言；真实 Windows
字体部署仍未验收。视觉截图：`serialforge-section-star_trail.png`、`serialforge-section-moonlit_ocean.png`、
`serialforge-section-sakura_night.png`。

## 架构、审查与简化记录

架构师 `019ff4d6-6e24-74a2-941a-b406376c5763` 批准 section owner；架构师
`019ff4dd-c011-7891-bcd4-5924116a0263` 批准水平 margin 为 0 的几何修正。独立 reviewer
`019ff4df-b76b-7c81-8cbd-2ea3ae392e5d` 等待窗口超时并关闭，未形成外部 findings；父代理完成六轴
review 与行为保持 simplification assessment。

本轮没有嵌入式 C/C++、MCU、RTOS、ISR/DMA、驱动、OTA 固件或安全升级实现；public-vendor-source applicability
为 N/A，不作 MISRA/ISO 26262/认证合规声明。未新增或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 交付产物

使用 `local-arch-117` 构建 onefile，并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical、
root、root-latest 均为 `48,031,424` bytes，SHA-256 为
`4488BDB5E31EEBF6C61D9BE4925C829E64A090E9C158396E6CA3B1A9F98400C0`；archive listing SHA-256 为
`ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过；签名
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
