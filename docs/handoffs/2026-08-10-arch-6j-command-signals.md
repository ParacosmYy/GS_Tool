# ARCH-6j：command signal/结果 owner 直连

日期：2026-08-10  
范围：删除 commands signal/结果投影的 MainWindow facade；不改变首屏 projection、batch 状态轨道、connection controls 或生命周期。

## 交付

- `commands.py` 内部直接调用 `on_command_batch_selection_changed()` 与 `render_command_batch_results()`，不再经由 window wrapper。
- `terminal.py` 的 batch combo 用 `partial(on_command_batch_selection_changed, window)`；bootstrap 对 ViewModel batch signals
  用 `partial` 接线，并直接调用 canonical owner 函数完成初始 batch catalog/snapshot 投影。
- 删除 `MainWindow._on_command_batches_changed`、`_on_command_batch_selection_changed`、`_on_command_batch_changed`、
  `_render_command_batch_results`；MainWindow 从 728 行降至 702 行。

## 架构审查

- 架构师 Luna max `019fe9ed-3ddb-7492-a242-1aab107c7e11` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。
- 父代理按已核对的最小 signal/owner 边界实现：bootstrap 只负责注入和初始调用，terminal 只负责 combo 接线，commands
  自持状态投影；没有引入动态 facade、controller 循环依赖或额外 timer。

## 验证

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (120 files <= 1000)
ARCH6J_COMMAND_SIGNALS                       pass
  facades=4-removed; bootstrap-projection=pass; combo-signal=pass
  themes=3; sizes=2; hscroll=0; motion-stop=pass; near-white=0; screenshot=pass
  screenshot: build/ui_review_arch6j_command_signals.png
PACKAGE_ARCH6J_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,787,662 bytes
  SHA256: `10591D3A3316F6775701331958F828C9DB8187F165821950DB31481C9EE531AB`
  provenance/archive/hash: pass; archive listing SHA256 `55C209CD1E9B0ECD92A27A863EDF767BF563F740DEADC1F359BA25A7FF56AC77`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏运行出现 PySide6 fonts 目录缺失提示，中文显示为方框；这不代表 Windows 打包环境字体缺失。未启动持续 GUI/EXE、真实
UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件；没有创建、修改或运行 unit test、mock、fixture、harness。项目为 Python/PySide6
桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。
