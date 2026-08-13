# ADR-0064：连接方式 section 标题与参数 panel 可见性同步

- 日期：2026-08-10
- 状态：accepted
- 范围：presentation / connection configuration

## 背景

连接页按 UART、网络和 BLE 分组展示参数。transport 切换时，运行时 controller 会隐藏不适用的参数 panel，
但 section 标题原先是 builder 内的局部变量，无法参加同一 projection，导致标题仍占位而内容消失，形成孤立标题行和视觉空白。
这与用户反馈的“链路链接一行白色/空白残留”直接相关，也会让不同 transport 的首屏层级不一致。

## 决策

1. `connection_builder.py` 将三个 `QLabel` 标题保存为 `window._uart_title`、`window._network_title`、`window._ble_title`，仍标记
   `role="section"`，只作为 presentation widget 引用，不成为业务状态源。
2. `connection_runtime.py:on_transport_changed()` 在既有 panel `setVisible` projection 旁对称同步标题：UART 仅显示 UART 组，BLE 仅显示 BLE 组，
   TCP Client/TCP Server/UDP/J-Link RTT 共享网络组。
3. 既有 `start_transport_panel_transition(window, active_panel)` 保持 active panel owner；不对标题新增 animation、timer 或 event bus，
   不修改 transport DTO、ViewModel、连接行为、焦点、Tab、accessibility 或 theme token。

## 被拒绝方案

- 在 MainWindow 增加新的“当前 section”状态：会复制 runtime projection，扩大 facade 和状态同步面。
- 为三个标题分别创建动画/定时器：会制造与既有 panel transition、MotionController 不一致的第二套动效时钟。
- 用 stylesheet 的 `:hidden` 或固定空白占位规避：无法保证标题与运行时 panel 状态一致，也不能修复布局空行。

## 验证

- `scripts/check.ps1`：pass；149 个 Python 文件均不超过 1000 行，theme token audit pass。
- `python -m compileall -q src`：pass。
- 真实 composition root + `QApplication`/Qt offscreen 向量：三主题 × 六 transport（UART/TCP Client/TCP Server/UDP/BLE GATT/RTT），
  标题与 panel parity、`role="section"`、startup hydration、lifecycle 均 pass。
- 未显示主窗口、未启动 EXE、未接入 UART/TCP/UDP/BLE/RTT/J-Link/OTA/硬件或网络；未创建或运行 test-only 资产。

## 复核记录

六角色与独立复核均按项目约束在源码修改前/后调用；等待窗口内均超时并关闭，未把超时视为通过。父代理完成 correctness、readability/simplicity、
architecture、security、performance 五轴复核。嵌入式 C/C++ 适用性：N/A。

