# UI-1.137 交接：工作区当前页上下文表面

日期：2026-08-11  
父代理：Codex；共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；本轮父代理为唯一写入者。

## 变更

- 新增 `src/serialforge/presentation/workspace_context_surface.py`，将真实 Tab index 投影为
  “当前页 · 链路配置 / 解析与遥测 / 命令管理 / 能力预览”。
- `WorkspaceShellBindings` 增加 typed `context_label` 引用；workspace builder 负责构造，
  workspace runtime 负责在既有 Tab change 边界同步。
- base/variant controls stylesheet 增加四态对称 selector，继续使用 ThemeSpec token，未引入
  白色 palette、第二套主题或局部时钟。

## 边界与复核

该表面只读、NoFocus、鼠标透明，不拥有导航、业务状态、timer、transport、OTA/AES/RTT/J-Link
依赖或设备 I/O；它的文案与 AccessibleDescription 是固定 bounded presentation catalog。
架构师调用 `019feddb-896c-7833-be05-c7d2052a767d` 在限定窗口内超时并关闭，父代理完成 owner、
依赖、index、QSS、accessibility、生命周期与简化复核；独立审查未返回。

三主题 × 980/1180 × 四 Tab 共 24 组真实组合根 vector 通过：context 文案/state/无障碍、
当前页 horizontal maximum=0、exact-white=0、near-white=0；1180×780 连接页和扩展页截图已
人工复核。offscreen 运行仍会出现已有的 Qt font-directory warning，但运行时系统字体选择
正常、中文可读性不受影响。

## 打包状态

onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.137` 构建并通过
provenance verify：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- root：[SerialForge.exe](../../SerialForge.exe)；root-latest：[SerialForge-latest.exe](../../SerialForge-latest.exe)
- 大小：`47,961,992` bytes；canonical/root/root-latest SHA-256：
  `7509738A8CD05655E8899497DCB67143A05C34999189426B3A2482EF1961829B`
- archive listing SHA-256：`12D69406BC04C668C3621D831004A420FFFD331AA3EB409479472BF10A93C2C4`
- source revision：`local-ui-1.137`；签名：`NotSigned`；`release_eligible=false`；
  `hardware_acceptance=not_run`

本轮未强制终止进程；原名覆盖已成功完成。

## 嵌入式 assurance

适用性：N/A；无嵌入式 C/C++ 或固件改动，无厂商目标资料、硬件刷写、部署或真实设备验收。
