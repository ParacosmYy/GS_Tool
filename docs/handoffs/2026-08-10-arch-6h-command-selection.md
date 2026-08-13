# ARCH-6h：typed command selector owner 收窄

日期：2026-08-10  
范围：删除 MainWindow 中两个只读 command selector facade；不改变发送/批量 gate、Qt combo data 或生命周期语义。

## 交付

- 新增 `src/serialforge/presentation/command_selection.py`，提供 `current_send_mode()` 和
  `selected_command_batch()` 两个 typed selector。
- `commands.py`、`connection.py`、`terminal_runtime.py` 直接导入 selector；保留 `CommandMode` fallback、
  `Qt.ItemDataRole.UserRole` 和无选中时的 `None` 语义。
- 删除 `MainWindow._current_send_mode` / `_selected_command_batch`；MainWindow 从 770 行降至 758 行，新文件 24 行。

## 架构审查

- 架构师 Luna max `019fe9e2-966e-75a3-a0a9-ba961238e0f9` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。
- 父代理按已核对的最小依赖图实现：selector 只依赖 domain DTO 和 Qt data role，不依赖 controller、ViewModel、transport 或
  MainWindow facade；三个调用方共享同一 typed 读取边界。

## 验证

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (120 files <= 1000)
ARCH6H_COMMAND_SELECTION                     pass
  facades=2-removed; mode=hex; batch-selection=pass; qt-data=preserved
  themes=3; sizes=2; hscroll=0; motion-stop=pass; near-white=0; screenshot=pass
  screenshot: build/ui_review_arch6h_command_selection.png
PACKAGE_ARCH6H_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,787,535 bytes
  SHA256: `EB8D47D5F0262CE5BD0E5DDE78BB3A2999EFE7EC7B4CAC1AB217AE1CAE481B6D`
  provenance/archive/hash: pass; archive listing SHA256 `55C209CD1E9B0ECD92A27A863EDF767BF563F740DEADC1F359BA25A7FF56AC77`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏运行出现 PySide6 fonts 目录缺失提示，中文显示为方框；这不代表 Windows 打包环境字体缺失。未启动持续 GUI/EXE、真实
UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件；没有创建、修改或运行 unit test、mock、fixture、harness。项目为 Python/PySide6
桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。
