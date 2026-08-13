# ARCH-130 / UI-1.202 自适应工作区 Tab 文案

日期：2026-08-12  
范围：`presentation/workspace_tab_surface.py`、`presentation/controllers/workspace.py`

## 本轮结果

修复真实字体下窄窗口工作区导航的文案裁切与拥挤。自定义 `AnimatedWorkspaceTabBar` 继续使用原生 `QTabBar`，依据所属 `QTabWidget` 的真实可用宽度在 full/compact/icon 三种可见投影间选择；父 controller 只安装 TabBar、保留原生 scroll-button fallback 并注入文案契约。路由、页面、signals、焦点、键盘顺序和 accessibility 语义没有迁移或复制。

## 验证证据

- 真实字体 `Microsoft YaHei UI`；三主题 × 12 宽度 × 4 页面 = `144` 行。
- `600/640/768/980/1120/1240px` 为 `full`；`360/420/480/520/546/560px` 为 `compact`；`546 -> 1240` 往返恢复 full。
- 每个页面逐一激活后，外层 horizontal scrollbar maximum 均为 `0`，page content 不超过 viewport；四项 full accessibility name 与 tooltip 恒等。
- 独立 `app.exec()` 采样：`240 frames / 113.954Hz`；仅作为共享 scheduler cadence 证据，不宣称真实显示器 120fps。
- Ruff、compileall、`scripts/check.ps1`、source-limit、theme-audit：pass。

## 审查与边界

架构师 `019ff646-9603-70e3-9d75-5e40bb286b44`：`APPROVE`。独立代码审查 `019ff6a6-0113-7dd0-b7fd-699b006f7f09`：`APPROVE`，Critical/Required=0。独立简化评估 `019ff6a6-200d-7680-bd14-36b7fe5c829d`：`APPROVE`，无必须简化项。最终 delta 只为 `_labels_fit()` 增加 `try/finally`，确保 `tabSizeHint()` 或样式计算异常时也恢复原始文案。

本轮无 embedded C/C++、固件、硬件或厂商目标约束改动，public-vendor-source applicability=N/A；未创建、修改或运行测试专用资产。验证为授权的非破坏性离屏/静态/打包检查；GUI/HIDPI/读屏/显示器合成、EXE startup、硬件链路、OTA、RTT/J-Link、签名发行仍为未运行项。

## 交付状态

源码和文档已通过本轮静态门禁；onefile 会以 `local-arch-130` 构建并覆盖仓库根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`。工程包不等同正式签名发行包，`release_eligible=false`、`hardware_acceptance=not_run`。
