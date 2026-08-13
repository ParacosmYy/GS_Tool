# ARCH-112 / UI-1.185 紧凑顶栏状态标题交接

日期：2026-08-12  
范围：`src/serialforge/presentation/controllers/workspace.py` 的紧凑顶栏视觉密度。  

## 结果

以实际 `_AdaptiveHeader.width()` 为依据，低于 `1120px` 时隐藏重复的视觉“连接状态”标题；保留
状态灯、当前状态值、状态组 accessible name/description、动态状态投影和主题/动效控制，宽屏恢复
标题。没有新增状态源、timer、QSS、业务、transport、OTA/AES/RTT/J-Link 或设备 I/O 依赖。

## 当前验证状态

`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH112_HEADER_DENSITY_PASS=pass`；`ARCH112_RESPONSIVE_PASS=pass`；
`ARCH112_ACCESSIBILITY_PASS=pass`；`ARCH112_LIFECYCLE_PASS=pass`；`package=pass`；
`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。

真实 Qt offscreen 三主题×`980×720`、`1040×720`、`1120×720`、`1240×820` 中，实际 header 宽度低于
`1120px` 时 caption hidden，宽屏恢复 visible；status cluster 约 `536px`（紧凑）/`598px`（宽屏），
`HMAX=0`，状态组 accessibility 保持。resize、`已连接` 状态投影、accessible description、暂停/
恢复/隐藏/关闭和唯一 MotionController timer 通过。

架构师 `019ff441-ac00-7e51-a0cd-a681d0dd2071` 与独立 reviewer `019ff443-dd0d-74c3-b425-ce42af1ec782`
均在等待窗口超时关闭，未形成外部结论；没有将超时写成 PASS。父代理完成 correctness、architecture、
security、performance、readability 五轴 review 与 behavior-preserving simplification assessment。
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

## 交付补录

静态门禁与 `compileall` 通过：179 个源码文件均不超过 1000 行，3 个主题、22 个 semantic tokens、
19 个 selectors 的 theme token audit 通过。`local-arch-112` onefile 已覆盖 canonical、根目录
`SerialForge.exe` 与 `SerialForge-latest.exe`；三者均为 `48,018,696` bytes，SHA-256 为
`2FACEA3043A66841DD1249228AC500EBCBEFC11D122D0C93B71EE35036F52C13`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，正式发行资格为 `false`，硬件验收为 `not_run`；本轮没有启动 EXE、接入硬件或
进行 OTA/RTT 实连。
