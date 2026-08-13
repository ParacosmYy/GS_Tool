# ARCH-70 / UI-1.143 连接 onboarding focus 交接

日期：2026-08-11  
父代理：Codex  共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 本轮目标

让未连接用户点击空终端的“打开链路连接”后直接看到完整连接配置，而不是先进入被压缩的
index 0 总览再手动点击“专注设置”。默认启动总览、连接业务语义与 120Hz 动效保持不变。

## 实现

新增 `workspace_runtime.open_connection_setup(window)`，并由 `bootstrap.py` 将
`TerminalEmptyState.connection_requested` 接入该 presentation-only 入口。它只选择 bounded
index 0、调用既有 `set_workspace_focus_mode(True)` 并把焦点交回真实 tabs；不创建连接、不读取
或修改 ViewModel/session、不写入 transport config。

ARCH-6z 的普通路由策略仍保持：协议/命令/扩展自动 focus，链路页默认总览；CTA 是唯一新增的
显式 connection focus 入口。

## 真实组合根验证

```text
CONNECTION_CTA_FOCUS_PASS viewport=514 lower_visible=[False, False, False] state=closed
CONNECTION_CTA_RETURN_PASS overview=115
TAB_ROUTE_COMPAT_PASS focus_restored
ADAPTIVE_LAYOUT_MATRIX_PASS themes=3 sizes=2 tabs=4
REDUCED_MOTION_STATIC_PASS focus_and_overview
MOTION_TARGET_PASS 120 [8, 9] surfaces=56
MOTION_COLD_CADENCE_PASS 105 frames / 1007ms = 104.3 FPS
compileall: pass
ruff: pass
scripts/check.ps1: pass
source line limit: pass (165 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, legacy_qss_literals=0)
```

离屏环境仍报告既有 PySide6 fonts directory warning，不代表 Windows 系统字体缺失。真实 Windows
GUI、HIDPI、读屏、设备 I/O、UART/网络/BLE/RTT/J-Link 和目标板硬件验收未执行。矩阵中只检查
可见 scroll area 的横向 maximum；隐藏页保留各自的纵向内容高度并不等于用户可见横向溢出。

## 复核与简化

- 架构师：Luna/max/Fast 只读线程已调用，等待后超时并关闭，未返回独立报告；随后再次调用的独立
  Luna/max/Fast 只读代码评审线程也在时限内超时并关闭；父代理未将任一超时视为通过。
- correctness：CTA 后 index 保持 0、focus 为 true、状态仍 `closed`；返回总览和普通 Tab route 均恢复。
- readability：一个命名入口承载 CTA intent，terminal empty 组件继续只发 signal，不知道 workspace policy。
- architecture：bootstrap 只接线，workspace runtime 负责 presentation 导航，focus transition 负责几何；无
  新 facade、timer、splitter、业务状态或外部依赖。
- security：没有输入、密钥、网络、持久化或依赖变更。
- performance：未增加 MotionController surface、QTimer 或帧分发；120Hz/8-9ms 既有不变量保持。
- independent review fallback：父代理在独立复核阶段重新读取两个源文件、bootstrap 调用链、既有
  `workspace_focus_transition`/typed bindings，并以 CTA、Tab 路由、低动效、主题/尺寸矩阵、静态门禁、
  产物哈希和 provenance 结果逐项复核；未发现需要追加简化或行为修复的项。

## 包交付

`local-arch-70` onefile 已生成并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`。

```text
PACKAGE_ARCH70_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,976,952 bytes
SHA256: 17F57F53EA2F3D49EB37E3E2581773F46F9528A243259D287E542EFE64FD3245
archive listing SHA-256: FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969
provenance: pass; source revision local-arch-70
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

## Assurance

本轮只改 Python/Qt presentation，public vendor source applicability 为 N/A；不涉及 embedded C/C++、
MCU、vendor SDK、RTOS、ISR/DMA、OTA firmware 或硬件，不宣称 MISRA/ISO 26262/认证合规。未创建或
修改单元测试、mock、fixture、harness 或 test-only asset。
