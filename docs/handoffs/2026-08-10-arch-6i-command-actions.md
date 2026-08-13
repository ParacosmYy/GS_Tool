# ARCH-6i：发送/批量 action owner 收窄

日期：2026-08-10  
范围：将 Qt signal payload 吸收到 commands owner，删除 MainWindow 的发送/批量动作 facade；不改变发送、批量、快捷键或生命周期语义。

## 交付

- `controllers/commands.py` 新增五个 owner action adapter，吸收 Qt checked/payload 后调用既有
  `send_current`、`new/edit/delete/run_command_batch`。
- `controllers/terminal.py` 使用 `partial(owner_action, window)` 连接发送按钮、returnPressed、批量按钮；
  `controllers/composition.py` 的 Ctrl+Enter 直接接入 `send_current_action`。
- 删除 `MainWindow._send_current`、`_new/_edit/_delete/_run_command_batch` 五个 facade；MainWindow 从 758 行降至 728 行。

## 架构审查

- 架构师 Luna max `019fe9e7-b9d2-7a03-ab38-8528f9730ea6` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。
- 父代理按已核对的最小 action 边界实现：Qt payload 只在 commands owner adapter 消费，dialog parent/ViewModel command
  仍由原 action 持有，composition/terminal 只负责 signal wiring，没有把 checked/payload 传入 domain/application。

## 验证

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (120 files <= 1000)
ARCH6I_COMMAND_ACTIONS                       pass
  facades=5-removed; qt-click=pass; payload-adapter=pass; batch-action-boundary=pass; shortcut-boundary=pass
  themes=3; sizes=2; hscroll=0; motion-stop=pass; near-white=0; screenshot=pass
  screenshot: build/ui_review_arch6i_command_actions.png
PACKAGE_ARCH6I_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,788,394 bytes
  SHA256: `4818D59EDE3F4984B1DBE6EE566A01BEC7805874CDEC2E70627B18F88D8FE5B8`
  provenance/archive/hash: pass; archive listing SHA256 `55C209CD1E9B0ECD92A27A863EDF767BF563F740DEADC1F359BA25A7FF56AC77`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏运行出现 PySide6 fonts 目录缺失提示，中文显示为方框；这不代表 Windows 打包环境字体缺失。未启动持续 GUI/EXE、真实
UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件；没有创建、修改或运行 unit test、mock、fixture、harness。项目为 Python/PySide6
桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。
