# ARCH-119 / UI-1.192 连接控制带响应式布局交接

日期：2026-08-12  
范围：`presentation/controllers/connection_builder.py` 内连接 transport/preset shell 的响应式几何 owner。

## 结果

连接控制带现在根据控件自身 sizing contract 选择常规或紧凑布局：980px 进入四行 compact 节奏，1180px
及以上保留常规三行节奏。传输方式、快速配置、连接/保存/删除动作、配置摘要和链路 rail 不再被迫挤在同一条
长横排；既有 QWidget identity、signals、callbacks、focus/accessibility、状态和唯一 120Hz MotionController
保持不变。UART 参数区、本轮 OTA/AES/RTT/J-Link contract 边界未改。

## 当前验证状态

`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`CONNECTION_THEME_RESPONSIVE=pass`（72 checks，0 failures）；`RESPONSIVE_LIFECYCLE=pass`（21 checks，0 failures）；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。

offscreen PySide6 font-directory warning 只影响截图字形；几何结果不等同真实 Windows GUI/HIDPI 或显示器 120fps。
未新增或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 架构、审查与简化记录

架构裁决 `019ff507-084e-7941-9583-f60b8fbda2ca` 为 `APPROVE`，指定 sizing-contract 断点；伸缩权修正由
`019ff50d-87f5-7110-98c9-75f7d35b03fc` `APPROVE`。较宽审查线程曾连续超时，未形成结论；独立 reviewer
`019ff50f-a11e-7042-b7e9-08f03167909e` 等待超时关闭，未形成外部结论；父代理完成六轴 review 与
behavior-preserving simplification assessment。本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

## 交付产物

使用 `local-arch-119` 构建 onefile 并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical、
root、root-latest 均为 `48,036,144` bytes，SHA-256 为
`CBED1FB08241C4D938796CED3F2AECF84C1A8BBFEC793A9832D489AAFBE495DD`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
