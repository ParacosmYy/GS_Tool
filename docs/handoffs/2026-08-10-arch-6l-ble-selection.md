# ARCH-6l：BLE selection owner 边界

日期：2026-08-10  
范围：删除 MainWindow 的 BLE 写入模式纯 helper facade；保持 BLE action、signal、normalize、连接 gate 与发送语义。

## 交付

- 新增 `presentation/ble_selection.py`，只读取 bounded BLE write-mode combo 和 characteristic capability；保留
  `Qt.ItemDataRole.UserRole`、enum fallback、`None`/空 capability 语义。
- `ble.py` 保留修改 combo 的 `normalize_ble_write_mode()` action，并直接导入 selector；`connection.py` 与 `commands.py` 直接
  消费 selector，不再通过 MainWindow 间接读取。
- 删除 `MainWindow._selected_ble_write_mode`、`_supported_ble_write_modes`、`_normalize_ble_write_mode` 三个 facade；MainWindow
  从 672 行降至 652 行。未改变 BLE 扫描、通知、读取、写入 gate 或跨模块 lifecycle wrapper。

## 架构复核与简化评估

- 架构师 Luna max `019fe9fb-ef39-74f3-83be-5150507a1231` 在等待窗口内超时并关闭，未把超时解释为 GO。
- 父代理确认 selector 只依赖 domain DTO 与 Qt combo data；normalize 仍由 BLE owner 控件动作负责。简化评估：移除 3 个纯
  facade，避免 connection/commands 读取 BLE controller 的展示内部，同时没有引入 ViewModel、transport 或设备句柄依赖。

## 验证

```text
python -m compileall / scripts/check.ps1       pass
source line limit                             pass (122 files <= 1000)
ARCH6L_BLE_SELECTION                          pass
  facades=3-removed; pure-selection=pass; mode=with_response; theme-switch=pass
  motion-stop=pass; near-white=0; screenshot=pass
  screenshot: build/ui_review_arch6l_ble_selection.png
PACKAGE_ARCH6L_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,787,928 bytes
  SHA256: `11B539A8A442A92C01C02FDCD7442E94547F4293500E0C0DC321495F2642C3F1`
  provenance revision `local-arch-6l`; archive listing SHA256 `83E4D141BBD5B7990D22742F91F30E9D13EEC35490092FA822B7C2A659F0DE60`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏环境提示 PySide6 fonts 目录缺失，中文可能显示为方框；这不等同于 Windows 字体验收。未创建、修改或运行 unit test、mock、
fixture、harness；未启动持续 GUI/EXE、真实 UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件。项目为 Python/PySide6 桌面应用，不含
嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。
