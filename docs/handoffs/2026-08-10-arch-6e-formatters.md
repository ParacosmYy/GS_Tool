# ARCH-6e：纯展示 formatter owner 收窄

日期：2026-08-10  
范围：删除 MainWindow 中仅用于端点/历史展示的两个静态 facade；不改变连接、回放、记录、Qt signal 或生命周期语义。

## 交付

- 新增 `src/serialforge/presentation/formatters.py`，集中 `endpoint_label()` 与 `history_label()` 两个无状态纯函数。
- `controllers/connection_runtime.py` 和 `controllers/terminal_runtime.py` 显式导入 formatter，不再经由 `MainWindow` 读取展示函数。
- 删除 `MainWindow._endpoint_label` 与 `MainWindow._history_label`；`lifecycle.py` 不再承载这两个与生命周期无关的 formatter。
- MainWindow 从 803 行降至 789 行；新文件 35 行，低于项目 1000 行门禁。

## 架构审查

- 架构师 Luna max `019fe9d1-1441-7702-b6fe-2388b0ac65a9` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。
- 父代理按已核对的最小依赖图实现：formatter 只依赖 domain DTO；两个 controller 直接依赖 formatter；没有引入
  `lifecycle` 与 `connection_runtime` 的反向循环依赖。

## 验证

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (118 files <= 1000)
ARCH6E_FORMATTER_BOUNDARY                   pass
  facade=removed; formatters=2; themes=3; sizes=2; hscroll=0
  motion-stop=pass; near-white=0; screenshot=pass
  screenshot: build/ui_review_arch6e_formatter.png
PACKAGE_ARCH6E_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,787,414 bytes
  SHA256: `40EDB44BF15AEC6F78295933BDAB379B731CC9B49C1343F1EF6F1BC7E9691D62`
  provenance/archive/hash: pass; archive listing SHA256 `E047A35E47FA63807D59624CBB16ADD3599CEC810590EC314F3CBD69C8688D7D`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏运行出现 PySide6 fonts 目录缺失提示，中文显示为方框；这不代表 Windows 打包环境字体缺失。未启动持续 GUI/EXE、真实
UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件；没有创建、修改或运行 unit test、mock、fixture、harness。项目为 Python/PySide6
桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。
