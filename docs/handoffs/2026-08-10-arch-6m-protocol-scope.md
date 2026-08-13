# ARCH-6m：protocol scope owner 边界

日期：2026-08-10  
范围：删除 MainWindow 的 protocol applicability 纯查询 facade；保持 protocol editor side effects 与 UART/TCP/UDP gate 语义。

## 交付

- 新增 `presentation/protocol_scope.py`，只读取 transport/history presentation facts，提供
  `parser_pipeline_supported()`、`derived_source_supported()` 和 `derived_source_unavailable_text()`。
- `protocol_config.py` 保留 editor enable/reset/status 等副作用；connection/lifecycle/derived/workspace/terminal 直接消费 query。
- 删除 `MainWindow._parser_pipeline_supported`、`_derived_source_supported`、`_derived_source_unavailable_text` 三个 facade；
  MainWindow 从 652 行降至 634 行。UART/TCP Client 派生和 UDP/raw-only gate 保持不变。

## 架构复核与简化评估

- 架构师 Luna max `019fea00-d301-7750-8ed7-3f7129155315` 在等待窗口内超时并关闭，未把超时解释为 GO。
- 父代理确认 query 模块无 UI 副作用、无 ViewModel/worker/transport 依赖；简化评估：移除 3 个纯 facade，保留所有协议 editor
  变更、reset 确认和状态投影在 canonical owner 中。

## 验证

```text
python -m compileall / scripts/check.ps1       pass
source line limit                             pass (123 files <= 1000)
ARCH6M_PROTOCOL_SCOPE                         pass
  facades=3-removed; pure-scope=pass; uart-gate=pass; udp-raw-only=pass
  theme-switch=pass; motion-stop=pass; near-white=0; screenshot=pass
  screenshot: build/ui_review_arch6m_protocol_scope.png
PACKAGE_ARCH6M_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,790,125 bytes
  SHA256: `9A82899684B8C162BB8AC3EBF727982CA52F1E4772081BAD9175A21D4E40DF66`
  provenance revision `local-arch-6m`; archive listing SHA256 `2C686B9AFB0B1E4C116CD0A51740BCA02C524E4F485E6F1942078B12C5F822BF`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏环境提示 PySide6 fonts 目录缺失，中文可能显示为方框；这不等同于 Windows 字体验收。未创建、修改或运行 unit test、mock、
fixture、harness；未启动持续 GUI/EXE、真实 UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件。项目为 Python/PySide6 桌面应用，不含
嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。

## Embedded R&D assurance gate

- 适用性：N/A；项目为 Python/PySide6 桌面应用，本轮没有嵌入式 C/C++/固件变更，也没有可适用的 MCU/SDK/RTOS 厂商一手约束。
- 独立复核：Luna max `019fea05-3ad1-7aa3-96f2-76e9ec38e3f1` 超时并关闭，未返回 findings；父代理完成只读依赖/行为/简化审计，
  超时不作为 GO。
- 简化评估：移除 3 个纯 MainWindow scope facade；`protocol_config.py` 继续拥有 editor enable/reset/status 副作用。
- 验证：`scripts/check.ps1`、123 模块导入且无 QApplication、ARCH6M offscreen、onefile provenance/archive/root hash 均 pass；
  EXE 启动、真实设备、硬件、正式签名发布未运行。
