# ARCH-6q 交接：Workspace shell typed bindings

日期：2026-08-11  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未创建/操作 worktree

## 变更

新增 `presentation/workspace_bindings.py` 的 frozen/slots `WorkspaceShellBindings`，
集中持有 workspace shell、QTabWidget、AnimatedWorkspaceTabBar、路线 beacon、滚动提示和
专注按钮。`controllers/workspace.py` 唯一创建 bundle，bootstrap 使用 `bundle.shell`；
workspace runtime、focus transition、焦点顺序、lifecycle、derived data 和 terminal runtime
通过 `workspace_bindings_for()` 读取，已移除散落的 `window._workspace_*` widget facade。

bundle 不携带 application/domain 状态、timer、transport、导航策略或业务 callback；header
主题初始化发生在 bundle 创建前时 helper 安全返回 None。

## 验证

- `ARCH6Q_WORKSPACE_BINDINGS_PASS WorkspaceShellBindings 4`；
- 三主题 × 980/1180 × 四 Tab 共 24 组 `ARCH6Q_WORKSPACE_VECTOR_PASS`；
- current page horizontal maximum=0，exact-white=0，near-white=0；
- 专注设置展开/恢复、Tab/route/scroll hint、共享动效生命周期保持；
- `compileall`、Ruff、source limit `160 files <= 1000`、`scripts/check.ps1`、theme token audit
  与 provenance verify 通过；
- 独立架构师线程 `019fedd1-fa7c-72a3-81c4-96ac4cb0f808` 与代码审查线程
  `019fedd6-db35-7d12-a58b-f7405b884fa9` 均在两个限定等待窗口内未返回并已关闭；父代理完成
  owner、依赖方向、生命周期、行为保持、性能和简化审查；
- 本轮无 embedded C/C++/firmware 改动，厂商资料与硬件验收 N/A/not-run；未创建、修改或运行
  unit test/mock/fixture/harness。

## 包产物

canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
source revision：`local-arch-6q`  
大小：`47,960,028` bytes  
SHA-256：`BACE302A92900CBC7A8D27D41D6BDF1082C70AE58F64F3F0D305AAF737701E63`  
archive listing SHA-256：`CFC788B805981C451920D9757F1FE48CD618C9049058B8ED52B20B70CF53EAC0`  
签名：`NotSigned`；`release_eligible=false`；hardware acceptance：`not_run`

根目录原名 `SerialForge.exe` 仍被 PID `46108`、`49236` 锁定，未强制终止；
`SerialForge-latest.exe` 已更新并与 canonical 字节一致。
