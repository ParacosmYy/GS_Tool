# ARCH-6d：连接动作 callback owner 收窄

日期：2026-08-10  
范围：继续收窄 `MainWindow` 的纯连接动作转发，不改变 UART/TCP/UDP/BLE/RTT 的行为或 transport 依赖。

## 交付

- `presentation/contracts.py` 新增 `ConnectionActionCallback` typed callback。
- `presentation/controllers/workspace.py` 直接组合 owner `connection_runtime.toggle_connection`，通过 callback 注入
  `connection_builder.build_connection_panel()`。
- `connection_builder.py` 只负责 QWidget 组合和 signal wiring；按钮不再依赖 `window._toggle_connection`。
- `connection_runtime.toggle_connection(window, *args, **kwargs)` 明确忽略 Qt signal payload，在 owner action 边界保持兼容。
- 删除 `presentation/main_window.py:MainWindow._toggle_connection`；MainWindow 从 811 行降至 803 行。

## 架构审查

- 架构师 Luna max `019fe9ca-cdd0-7932-a3ef-511485014748`：等待、立即返回请求均未在窗口内返回结论，已关闭；未把超时解释为批准。父代理按已审计的最小 callback 方案实现，保留唯一写入权。

## 验证

```text
targeted compileall / ruff                     pass
ARCH6D_CONNECTION_CALLBACK                     pass
  - MainWindow._toggle_connection absent
  - actual create_application/create_main_window composition
  - 3 themes
  - 980x680 / 1180x780
  - shared MotionController pause/disable/close stop
ARCH6D_SCREENSHOT                              pass
  build/ui_review_arch6d_connection_callback.png
scripts/check.ps1                              pass (117 files <=1000)
onefile package                                pass
```

包交付：`dist/release/0.1.0/core/onefile/app/SerialForge.exe` 已覆盖根目录 `SerialForge.exe`，两者字节一致；大小
47,785,910 bytes，SHA256 `674AF78618BEFE9C61618FFF4B2FECE39C617D067C13CEFC3F7DC590F1A1B8F3`。provenance/archive/hash
通过；签名 `NotSigned`，release eligible `false`，hardware acceptance `not_run`。

没有创建、修改或运行 unit test、mock、fixture、harness；没有启动真实 GUI/EXE 持续进程、设备、服务或硬件。项目为 Python/PySide6 桌面应用，不含嵌入式 C/C++/固件；嵌入式企业工作流厂商源适用性为 N/A。
