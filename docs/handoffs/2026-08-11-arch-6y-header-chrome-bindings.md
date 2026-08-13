# ARCH-6y / UI-1.141 Header Chrome typed binding

日期：2026-08-11  
状态：源码、静态检查和真实组合根验证完成；onefile/root 覆盖待完成

## 变更

新增 `presentation/chrome_bindings.py` 的 frozen/slots
`HeaderChromeBindings`，由 `controllers/workspace.py` 在 header 的 status、品牌、
动效和主题控件全部创建后、首次 `on_theme_changed()` 前唯一组装。

`lifecycle.py` 和 `composition.py` 已通过 `header_chrome_bindings_for()` 消费：

- status cluster、context/source/state label 与 state indicator；
- low-motion/pause controls；
- theme combo、theme palette swatch；
- brand mark 与 signal field。

`PresentationPreferences`、`MotionController`、theme policy、`app_root`、error bar、
status footer、ViewModel、业务 callbacks、timer 和策略没有进入 bundle。builder 动态
字段只用于构建/接线阶段。

## 验证

```text
HEADER_CHROME_RUNTIME_PASS HeaderChromeBindings surfaces 56
MOTION_TARGET_PASS 120 [8, 9]
HEADER_CHROME_LAYOUT_PASS 980 720
HEADER_CHROME_LAYOUT_PASS 1240 820
HEADER_CHROME_THEME_PASS 0/1/2
scripts/check.ps1: pass
source line limit: 165 files <= 1000
theme token audit: 3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0
```

header status/motion/theme geometry remains non-overlapping at 980px; all four workspace
pages retain horizontal scrollbar maximum 0. Motion surface IDs are unique after replacing
the old dynamic-field fan-out entries.

## assurance gate

- public source applicability：N/A；本轮只改 Python/Qt presentation，不涉及 MCU、embedded
  C/C++、vendor SDK、RTOS、ISR/DMA、OTA firmware 或硬件。
- architect review：Luna/max/Fast 架构师线程已调用，连续等待后超时并关闭，未返回报告；
  父代理根据实际 call chain 完成最小字段、owner、初始化顺序和不变量复核。
- simplification assessment：复用现有 workspace builder/lifecycle/composition 边界，
  只新增一个 Qt-reference bundle；没有新增全局 facade、状态源、timer、动态 registry，
  `app_root`/error/footer 特意不纳入以避免 bundle 膨胀。
- authorized non-destructive validation：compileall、Ruff、`scripts/check.ps1`、offscreen
  组合根、三主题、两尺寸、四页滚动和 motion target 已执行；真实 Windows GUI/EXE 启动、
  HIDPI、读屏、UART/网络/BLE/J-Link/目标板和硬件验收未执行，原因是未授权。

## 包交付

`local-arch-6y` onefile 已生成并覆盖根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`。canonical/root/root-latest 均为 `47,978,616` bytes，SHA-256
`842BF2981DBC4EAFC47C30819904DD290BC087A16EB1565E633FD0C5C4028C8C`，archive listing
SHA-256 `FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969`；签名
`NotSigned`，`release_eligible=false`，硬件验收 `not_run`。
