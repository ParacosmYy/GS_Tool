# ARCH-6w / UI-1.139 命令批处理绑定边界

日期：2026-08-11  
状态：源码、静态门、真实组合根 vector、onefile 与根目录覆盖完成  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`  
父代理：Codex；本轮唯一写入者

## 交付

新增 `src/serialforge/presentation/command_bindings.py`：

- `CommandBatchControlBindings` 使用 `frozen=True, slots=True`；
- `bootstrap.py` 在 `workspace.py` 构建命令页后唯一组装；
- commands、connection、command selection、composition、lifecycle 通过
  `command_batch_bindings_for()` 消费；
- `terminal.py` 仅保留构建阶段动态字段，跨 controller 的批处理 widget 读取清零；
- bundle 不含 batch catalog/snapshot、ViewModel、执行策略、timer、callback 或 transport handle。

行为保持范围：批处理选择、编辑、删除、执行、停止、空态/结果表可见性、Tab 顺序、
共享 MotionController、主题和 OTA/debug contract-only/attach-only 边界不变。

## 验证

```text
COMMAND_BINDINGS_STATIC_PASS
COMMAND_BINDINGS_RUNTIME_PASS CommandBatchControlBindings
COMMAND_BINDINGS_LAYOUT_PASS
VISIBLE_SCROLL_LAYOUT_PASS (all 4 workspace pages, hmax=0 at visible page)
MOTION_240MS_FRAME_EVENTS=18 (event-loop sample; timer target remains 120Hz)
source line limit: pass (164 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
scripts/check.ps1: pass
compileall: pass
ruff: pass
```

离屏环境报告既有 PySide6 fonts directory warning；系统字体注册路径可用，不能把该 warning
解释为 Windows 发行环境缺少字体。未创建、修改或运行 unit test、mock、fixture、harness 或
其他测试专用资产；未执行可见 GUI、EXE 启动、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、刷写、
部署或硬件操作。

## 包交付

```text
source revision: local-arch-6w
canonical: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe
root-latest: SerialForge-latest.exe
size: 47,974,086 bytes
SHA256: 3BCACA7C90A39C23FF7E4C4605F021E463174AF2BE8032C75496C6E10A0F6327
archive listing SHA256: 60CADFDE893442CAC3AE9797A0EA53F8233813B1D4D66C458DEE413E8474BD0C
provenance: pass
signature: NotSigned
release_eligible: false
hardware_acceptance: not_run
canonical/root/root-latest: byte-identical
```

## 架构师与 assurance gate

架构师 Luna/max/Fast 线程 `019ff141-f6a2-74e2-b885-57d12f3c11fa` 已按要求调用，限定为只读审查，
等待窗口内未返回独立报告，已安全关闭；父代理完成依赖方向、初始化时序、Qt lifetime、
accessibility、动效单时钟、性能和行为保持型简化审查。

本轮为 Python/PySide6 presentation 变更：

- public embedded vendor source applicability：N/A；
- embedded C/C++/firmware review：N/A；
- simplification assessment：父代理通过，无业务行为改变；
- authorized non-destructive validation：静态检查、离屏真实组合根、布局 geometry、动效 cadence probe、provenance 和 hash；
- 不作 MISRA、ISO 26262、ASIL、ASPICE 或正式发行合规声明。
