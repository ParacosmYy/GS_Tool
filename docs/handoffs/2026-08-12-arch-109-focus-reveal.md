# ARCH-109 / UI-1.182 交接：专注/总览首帧可读淡入

日期：2026-08-12

## 结果

修复 980×720 紧凑窗口从“专注设置”返回“总览”时的视觉空白：shell 布局仍先结算最终位置，
但实时观测、终端、发送 surface 不再从完全透明开始；它们由 `workspace_focus_transition.py`
通过 `_REVEAL_START_OPACITY = 0.82` 从可读首帧淡入到 `1.0`。用户可以在整个 220ms 过渡中看到
组件，避免误以为组件被挤走或动画丢帧。

## 边界

- 仅修改 `src/serialforge/presentation/workspace_focus_transition.py`；新增一个命名常量并改变
  一个现有 reveal 起点。
- 不改变布局 geometry、`maximumHeight` policy、业务状态、ViewModel、连接/终端/记录语义、
  scroll owner 或 OTA/AES/RTT/J-Link contract-only/attach-only 边界。
- 不新增 timer、singleShot、processEvents、线程、paint loop 或独立动画时钟；继续复用唯一
  `MotionController` 和 `MotionDrivenAnimationGroup`。

## 证据

- `scripts/check.ps1`：pass；source limit `179 files <= 1000`，theme token audit pass，ruff pass。
- Qt offscreen `980×720`、`1240×820`：overview reveal 首帧 opacity `0.82`，约 110ms 为
  `0.976~0.977`，约 220ms 为 `1.0`；三个 lower surface geometry 全程稳定、可见。
- 反向 focus：三个 lower surface 最终 visible false、height 0，页签 viewport 恢复。
- 三主题截图：`star_trail`、`moonlit_ocean`、`sakura_night` 均通过，无白色断层或横向溢出。
- 980/1240 四 workspace：Tab 切换、route/context/scroll hint geometry 通过，横向 scrollbar 为 0。
- 用户路径生命周期：低动效、暂停、隐藏/恢复、关闭通过。
- MotionController：`frames=77`、均值 `8.322ms`、p95 `9.919ms`、有效约 `120.16Hz`，target
  `120Hz`、timer `8ms`。
- 本轮没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only asset。

## 审查

架构师调用 `019ff413-0974-7210-8c9b-c228b1bbbd8b` 与独立 reviewer 调用
`019ff418-272d-7232-b5d8-1a821d76ed40` 均在等待窗口内超时关闭，随后关闭线程，未形成外部
结论，未伪造 PASS。父代理完成 correctness、architecture、security、performance、readability
五轴 review 与 behavior-preserving simplification assessment。

本轮没有嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A；不作 MISRA、ISO 26262、
ASIL、ASPICE 或认证声明。EXE startup、真实 Windows 可见窗口、高刷新显示器/HIDPI、高负载、硬件
连接、OTA/RTT 实连和签名验收仍需授权环境验证。

## 交付

本轮使用 `local-arch-109` onefile 覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,019,597` bytes，SHA-256 为
`35DE8906B0838D29562F85E2518035C64DA5B69FAA65F9AFA7BCB8B065ED9DD7`，archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
