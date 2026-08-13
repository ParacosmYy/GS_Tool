# ARCH-115 / UI-1.188 短页面垂直节奏交接

日期：2026-08-12  
范围：`ResponsiveScrollArea`、共享 `scroll_page()` 入口、短页居中与长页原生滚动。

## 结果

共享滚动容器现在按内容高度自适应垂直节奏：短页面居中，长页面顶部对齐并保留原生滚动。连接页
不再顶部堆叠，focus shell 内下方不再形成无语义的视觉重心；协议页和扩展页的滚动起点、横向禁用策略
保持。命令管理的 `CommandBatchEmptyState` 继续使用自身 `Expanding` canvas。

## 当前验证状态

`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`RESPONSIVE_SCROLL_MATRIX_PASS=pass`（217 checks，0 failures）；`RESPONSIVE_LIFECYCLE_MATRIX_PASS=pass`
（752 checks，0 failures）；`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。

Qt offscreen 有已知 PySide6 font-directory warning，截图文字在该环境中显示为方框；这不代表真实
Windows 字体部署已验收。截图几何与滚动行为断言均通过。

## 架构、审查与简化记录

架构师 `019ff4ad-186a-7fb2-8a28-83e47bf9fa20` 有条件批准 helper 边界；架构师
`019ff4b1-dc64-76c1-85ec-0c3858663ba7` 批准移除不存在的 viewport resized 信号；两项要求均已落地。
本轮另有架构审查调用 `019ff4a3-fab4-7aa0-a2a3-5e3e95c7a07e`、`019ff4a6-dad2-7121-b654-43d0d4916171`、
`019ff4a9-758c-7c11-b353-465866dfd3b9` 超时关闭，未形成外部结论，未将超时记为通过。独立 reviewer
`019ff4b4-c995-7fb0-9ad2-c0b403706716` 超时关闭，未形成外部 findings。父代理完成 correctness、
architecture、performance、lifecycle/accessibility、security、readability 六轴 review 与行为保持
simplification assessment。

本轮没有嵌入式 C/C++、MCU、RTOS、ISR/DMA、驱动、OTA 固件或安全升级实现；public-vendor-source
applicability 为 N/A，不作 MISRA/ISO 26262/认证合规声明。

## 交付产物

`local-arch-115` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`。
三者均为 `48,028,531` bytes，SHA-256 为
`8E98DB669C51C64813B701015A9DC2E2C9C7BAE4729025278BEECAF218713924`；archive listing SHA-256 为
`CD3C60D336971C88071C3A0D1C3B74D6AD090D1D2704F753C13B058E236913A0`，provenance verify 通过。
签名为 `NotSigned`，manifest `release_eligible=false`，`hardware_acceptance=not_run`。
