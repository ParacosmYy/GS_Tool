# ARCH-6b 交接：terminal runtime owner-module contract

日期：2026-08-10  
范围：MainWindow facade 渐进收窄的 terminal runtime 微切片。  
父代理：Codex；父代理是本轮唯一写入者。  
架构师：Luna max `019fe948-b8bd-7580-bcca-91c99f55689c`，GO（仅限本微切片）。  
独立复核：Luna max `019fe94e-d033-7500-be30-d82e90a84034`，GO，Critical=0、Required=0、Optional=1。  

## 变更

- 删除 `presentation/main_window.py` 的 `MainWindow._current_entry` 和 `MainWindow._render` 两个仅供 terminal
  runtime 自身调用的转发 facade，并删除对应 imports。
- `controllers/terminal_runtime.py`：`save_current_quick()` 直接调用同模块 `current_entry(window)`；
  `rerender_preview()` 直接调用同模块 `render(window, payload)`。
- 保留其他 terminal runtime wrapper，因为它们仍承载 Qt signal/cross-workspace/lifecycle 接线；不把完整 `window._*`
  访问伪装成一次性 DTO 迁移。

## 架构边界与行为保持

这是 owner-module contract 的最小迁移：canonical helper 与调用者在同一 owner module，删除没有外部调用价值的
MainWindow facade。没有改动异常处理、command payload 编码、preview render、滚动到底部、terminal empty state、Qt parent、
Tab、objectName、timer 或 close/hide/show/minimize 生命周期。更大范围 protocol/derived/workspace/terminal runtime DTO
迁移被架构师明确留到后续切片。

## 验证

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (110 files <= 1000)
ARCH6B_TERMINAL_OWNER                            pass (old facade=0, direct owner calls)
ARCH6B_COMPOSITION                               pass (980/1180, 3 tabs, hscroll=0)
```

`main_window.py` 当前 939 行，`terminal_runtime.py` 494 行。未创建、修改或运行 unit test、mock、fixture、harness；未启动
持续 GUI/EXE、HIDPI、读屏、性能或真实 UART/TCP/UDP/BLE/RTT/J-Link 硬件。该切片不涉及嵌入式 C/C++、固件或 MCU，
embedded-enterprise-workflow 的厂商源适用性为 N/A；不声称 MISRA、ISO 26262、硬件或认证合规。

## 交付

ARCH-6b 代码和文档已完成，onefile 已重新生成并覆盖根目录副本。

```text
canonical/root byte match                          pass
SHA-256                                             6F03E52E252FEA0A46C81CE37D233984A5A1BEB9CB9CE6B50097BE5B4CE3729D
size                                                47,755,831 bytes
manifest hash/size                                  pass
archive connection_runtime/terminal_runtime        present
signature                                           NotSigned
release_eligible                                   false
hardware_acceptance                                not_run
```

canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；根目录覆盖：[SerialForge.exe](../../SerialForge.exe)。
未启动 EXE 或进行正式 GUI/HIDPI/真实硬件验收。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
