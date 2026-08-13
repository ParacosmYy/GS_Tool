# ARCH-6c composition/workspace owner-module contract

日期：2026-08-10  
范围：继续收窄 `MainWindow` 对 presentation composition/workspace 的纯转发依赖，保持 Qt 行为和既有业务回调不变。

## 实现

- `presentation/main_window.py` 删除 20 个纯 facade：header、workspace Tab、滚动页、connection/protocol/send/error/terminal page
  builder、bounded combo/timeout helper、shortcut/tab-order 安装，以及 workspace transition/tab-change 转发。
- `controllers/bootstrap.py` 直接调用 shell/page/shortcut owner。
- `controllers/workspace.py` 直接组合既有 connection/protocol/send builder，并用自身函数连接 Tab change 与 workspace transition。
- `controllers/lifecycle.py` 直接调用 workspace motion/stop/tab-change helper。
- `controllers/connection_builder.py` 直接调用 `composition.py` 的 `enum_combo()`/`timeout_spin()`。

未引入动态注册、mixin、`MainWindowContext` 或无限制 callback map；业务 ViewModel signal/callback wrapper 未在本切片扩大迁移面。

## 架构与复核

- 架构师 Luna max `019fe9aa-9b96…` 在等待窗口内未返回，随后关闭；父代理按已审计的最小 owner-module 边界完成实现。
- 独立审查 worker Luna max `019fe9af-5a33…` 在等待窗口内未返回，随后关闭；不将其写成独立 GO。父代理完成
  correctness/readability/architecture/security/performance 五轴审阅，未发现 Critical/Required 项。

## 验证

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (115 files <= 1000)
ARCH6C_COMPOSITION_FINAL                         pass (dpr=1.50, tabs=3, scrolls=3, hscroll=0, facades=0, lifecycle-stop=pass)
ARCH6C_SCREENSHOT                                pass (build/ui_review_arch6c_composition.png)
```

验证确认删除的 facade 在 production presentation 中无调用，3 个 Tab/滚动页正常组装，1.5x HIDPI 下无横向滚动，暂停/隐藏时
workspace transition 静态停止。截图已人工查看。offscreen 环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 运行时字体缺失。

未创建、修改或运行 unit test、mock、fixture、harness；未启动持续 GUI/EXE、真实焦点/读屏、动效性能或 UART/TCP/UDP/BLE/RTT/J-Link
硬件。

## 包交付

```text
PACKAGE_ARCH6C                                pass
artifact size                                  47,775,103 bytes
canonical/root SHA256                          7E15E2B7A0887D0A91F7D968942221BB7CADCC0050A1292978F69B4C986111EF
archive module verification                    pass (main_window, bootstrap, workspace, lifecycle, connection_builder, terminal_surface)
signature / release_eligible                   NotSigned / false
hardware_acceptance / vendor_binary_matches     not_run / 0
```

canonical `dist/release/0.1.0/core/onefile/app/SerialForge.exe` 已覆盖根目录 `SerialForge.exe`，两者 hash/size 完全一致。
