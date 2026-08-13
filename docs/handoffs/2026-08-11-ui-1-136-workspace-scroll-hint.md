# UI-1.136 交接：工作区滚动可发现性提示

日期：2026-08-11  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未创建/操作 worktree

## 交付

新增 `presentation/workspace_scroll_hint.py:WorkspaceScrollHint`，把当前工作区
`QScrollArea` 的 vertical scrollbar range/value 投影到既有 `workspaceRouteStrip`：

- 顶部：`↓ 向下查看`；
- 中部：`↕ 上下滚动`；
- 底部：`↑ 返回顶部`；
- 无溢出：`内容已全部显示`。

`workspace.py` 只负责组装，`workspace_runtime.py` 在 Tab 切换时解绑旧 scrollbar
signal 并绑定当前页。提示 NoFocus、鼠标透明，动态更新 tooltip 与
AccessibleDescription；不新增 application/domain 状态、DTO、timer、导航模型、设备
I/O 或第二套滚动策略。base/variant QSS 仅复用既有 semantic token。

## 架构与审查

架构师线程 `019fedc5-ee6c-7a43-8178-9ee97bb0559d` 在两个限定等待窗口内未返回，
已关闭，未计为独立通过。独立代码审查线程 `019fedcb-bcd4-7fe0-abb5-ff663995cd9b`
同样在两个等待窗口内未返回，已关闭。父代理完成 owner、signal 生命周期、Tab 边界、
Qt API、无障碍、主题 token、行为保持、性能和简化审查；未修改业务层。

## 验证

- `scripts/check.ps1`：pass；source limit `159 files <= 1000`；theme token audit pass；
- `compileall`：pass；Ruff：pass；
- 三主题 × 980/1180 × 四 Tab 共 24 组：当前页 horizontal maximum=0，顶部/中部/底部
  state transition pass；
- 主题像素审计：exact-white=0，near-white=0；截图位于
  `build/ui_review_ui136_<theme>_tab<index>.png`；
- 未创建、修改或运行 unit test/mock/fixture/harness；未运行硬件、刷写、部署或正式 GUI
  启动验收；
- 嵌入式 C/C++/固件适用性：N/A，本轮仅 Python/PySide6 presentation 代码；无厂商目标
  资料适用，无 MISRA/ISO/硬件合规声明。

## 包产物

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-ui-1.136`  
大小：`47,957,987` bytes  
SHA-256：`D028C8D939A57A64BCD991CF6E0AAF404883450307974049566891560696DACC`  
archive listing SHA-256：`B9F1EB9352EC1F155CE7E8F334DD34FCE442221281D32C4CBE59AE3276B7FE9E`  
provenance：pass；签名：`NotSigned`；`release_eligible=false`；hardware acceptance：`not_run`

根目录 `SerialForge.exe` 仍被 PID `46108`、`49236` 锁定，未强制终止进程；本轮已额外
生成根目录可直接运行的 [`SerialForge-latest.exe`](../../SerialForge-latest.exe)，与 canonical
字节一致。关闭旧实例后，再将 canonical 覆盖为根目录 `SerialForge.exe`。
