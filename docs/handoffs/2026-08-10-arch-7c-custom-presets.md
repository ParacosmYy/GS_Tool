# ARCH-7c：自定义连接 preset editor、合并与显式保存

日期：2026-08-10  
范围：为 ARCH-7b 的安全 catalog store 增加用户自定义连接配置管理，不改变 transport/runtime 语义。父代理是唯一写入者，工作目录为 `D:\Workplace\Agent_Workplace\SerialForge`，未创建或操作 worktree。

## 交付结果

- 新增 `src/serialforge/presentation/connection_preset_editor.py`：只编辑名称/备注，自动生成 custom key；对话框继承 parent 主题，不接触密钥、设备句柄、BLE identity 或自动连接。
- `controllers/connection_presets.py` 增加当前表单到 typed `ConnectionPreset` 的 snapshot、custom save/delete action；UART 的 StrEnum 控件值显式转换回 domain enum，避免 Qt QVariant 字符串化破坏 DTO 校验。
- `connection_preset_surface.py` 只接收明确 `QComboBox` 并负责 custom/builtin combo 刷新；`controllers/connection_builder.py` 负责
  combo 组装和保存/删除动作按钮；`controllers/connection.py` 按活动连接、历史回放和 custom selection gate 按钮，controller 之间无直接导入。
- `composition.py` 创建一个 store 实例，`bootstrap.py` 和 `MainWindow` 只负责注入；builder 不读取 QSettings。
- `connection_presets.py` 增加 immutable builtin key 集合及 merge helper；内置 key 不可覆盖，catalog 总量仍不超过 16。
- `connection_preset_store.py` 只持久化 custom 项，读取时与 immutable builtin 合并；`save()` 返回 `bool`，保存失败 fail-open。删除失败不改变当前 catalog/UI，保存失败允许本会话使用但明确提示。

## 架构边界记录

- 架构师 Luna max `019fe9b6-df24…`：超时，未返回结论；随后关闭。
- 架构师 Luna max `019fe9b9-e5b7…`：超时，已发送立即返回请求，仍无结论；随后关闭。
- 主代理依照事先审计边界继续，未把超时记录解释为批准；父代理保留唯一写入权。
- 独立质量复核 Luna max：`019fe9bf-749c-73a2-bf6d-6c6a40b3cb0e`，两次等待与立即返回请求均超时，已关闭，未返回结论；不伪造 GO，父代理完成本地边界复核。
- 后续架构复核 Luna max `019fe9c4-1186-7313-ade3-c9ce81c6e9b7`：`ARCHITECTURE_GO`；建议已整合：去掉 combo
  重复初始化，让 `connection_preset_surface.py` 接收明确 `QComboBox`，消除 controller-to-controller 依赖。

## 验证

```text
targeted compileall / ruff                     pass
scripts/check.ps1                              pass (117 files <=1000; 3 themes)
ARCH7C_UI_FINAL                                pass
  - actual create_application/create_main_window composition
  - 3 themes
  - 980x680 and 1180x780
  - custom snapshot/apply and button gate
  - QSettings roundtrip and malformed payload builtin fallback
ARCH7C_SCREENSHOT                              pass
  build/ui_review_arch7c_custom_presets.png
onefile package                                pass
```

offscreen 环境提示 PySide6 fonts 目录缺失，截图中文显示为方框；这不等同于 Windows 字体缺失。没有创建、修改或运行 unit test、mock、fixture、harness；未启动持续 GUI/EXE、真实 UART/TCP/UDP/BLE/RTT、J-Link、OTA 或硬件。

## 包交付

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root:     SerialForge.exe (byte-identical)
size:     47,785,521 bytes
SHA256:   3ABE60DB76C0796A465167BD7EE38E0FADAE4594CB4D2DDC846EAD00AB644B80
provenance/archive/hash: pass
signature: NotSigned
release_eligible: false
hardware_acceptance: not_run
```

## 简化评估与嵌入式门禁

本切片没有新增可安全删除的重复状态源：editor 只持有临时文本，controller 只生成一次 immutable DTO，store 只负责持久化。未修改嵌入式 C/C++、固件、BSP/HAL、RTOS、ISR/DMA、bootloader 或硬件驱动；`mcu`、`embedded-enterprise-workflow`、`embedded-code-review-simplifier` 的厂商资料适用性为 N/A，不宣称 MISRA/ISO 26262/认证或硬件验收。

## 后续

- 独立复核子代理未返回结论；不伪造复核 GO，后续若需要更高保证应在可用运行时重新发起独立审查。
- onefile 包已重建并覆盖仓库根目录；当前包已完成 SHA256、archive listing、provenance 校验，但仍受未签名/未授权硬件限制。
- ARCH-8 仍保持 OTA/XMODEM/YMODEM/TFTP、AES-GCM/CCM、RTT/J-Link contract-only/attach-only，等待目标资料和授权硬件证据。
