# ARCH-6k：motion callback 直连与 workspace runtime 边界

日期：2026-08-10  
范围：删除 MainWindow 的主题/动效纯转发入口；修复 workspace 与 lifecycle 的循环依赖；保持 UI 状态和关闭清理语义。

## 交付

- `workspace.py` 的 MotionController frame、低动效、暂停动效和主题 combo 直接用 `partial(owner, window)` 接线；主题首屏
  直接调用 `on_theme_changed(window)`。
- 删除 `MainWindow._on_motion_toggled`、`_on_theme_changed`、`_on_motion_pause_toggled`、`_on_motion_frame`、
  `_set_data_activity_motion` 五个纯 facade；MainWindow 从 702 行降至 672 行。
- 首次离屏组装发现 `workspace.py -> lifecycle.py -> workspace.py` 循环导入。新增
  `controllers/workspace_runtime.py`，收拢 Tab 可见性、派生 surface 挂起、workspace motion policy 和一次性淡入过渡；
  `workspace.py` 只组装/接线，`lifecycle.py` 只消费 runtime helper。
- `lifecycle.py`、`terminal_runtime.py` 直接调用 `set_data_activity_motion()`；不再通过 MainWindow 隐式转发。

## 架构复核与简化评估

- 本轮架构师 Luna max `019fe9f2-5c68-7061-b5bb-b38046c5b141` 与 Terra max
  `019fe9f5-6d49-78d3-893d-fae5298c3be5` 均在等待窗口内超时并关闭，未把超时解释为 GO。
- 父代理保留完整 ImportError 证据，沿依赖图定位环依赖后采用独立 runtime 模块；没有使用延迟动态导入、`__getattr__`、mixin、
  无限 callback map 或重复状态源。简化评估：移除 5 个纯 facade，并把稳定的 Tab/过渡运行态放入单一 owner，行为保持。

## 验证

```text
python -m compileall / scripts/check.ps1       pass
source line limit                             pass (121 files <= 1000)
ARCH6K_MOTION_LIFECYCLE                       pass
  facades=5-removed; runtime-boundary=pass; theme-switch=pass; frame-signal=pass
  motion-toggles=pass; near-white=0; screenshot=pass
  screenshot: build/ui_review_arch6k_motion.png
PACKAGE_ARCH6K_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,789,020 bytes
  SHA256: `39B258DDB9DB43FFF15F13C744EAA9731E48DE8DE3A49A4945724FE10C96F7E4`
  provenance revision `local-arch-6k`; archive listing SHA256 `6483837FF1A50CDEE49E5B7DE49BC29FC607F96B808897AC641E78AA1A17730A`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏环境提示 PySide6 fonts 目录缺失，中文可能显示为方框；这不等同于 Windows 字体验收。未创建、修改或运行 unit test、mock、
fixture、harness；未启动持续 GUI/EXE、真实 UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件。项目为 Python/PySide6 桌面应用，不含
嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。
