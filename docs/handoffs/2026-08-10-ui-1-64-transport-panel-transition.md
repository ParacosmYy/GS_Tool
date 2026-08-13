# SerialForge UI-1.64 交接：Transport Panel Transition

日期：2026-08-10  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 交付结果

连接方式切换后，UART/TCP Client/TCP Server/UDP/BLE GATT/J-Link RTT 的当前配置面板获得
一次性 160ms fade。它只改善 presentation 过渡，不移动布局、不改变焦点、不自动连接，
也不把动画状态当成连接状态。

动画生命周期如下：

- `connection_runtime.py` 先完成既有 `setVisible()` 和字段显隐，再把目标 QWidget 交给 transition owner；
- `transport_panel_transition.py` 最多拥有一组 `QGraphicsOpacityEffect/QPropertyAnimation`；
- `motion_policy.py` 统一低动效/暂停偏好，`workspace_runtime.py` 和新 transition 共同复用；
- 主题切换、低动效、暂停、隐藏、最小化、关闭和快速切换都回到无 effect 的静态 panel；
- 目标 panel 已有其他 graphics effect 时不接管，避免覆盖其他 presentation owner。

## 评审记录

产品、架构、UI 设计、开发、验证、打包/流程六个 Luna/max 角色均在源代码修改前调用，
对应 IDs 为：

`019feb4b-2104-7222-b9bd-140d78902bcf`、`019feb4b-2151-7510-ae3b-4d9d7b9c8420`、
`019feb4b-219b-73e2-a25b-008d531c54fb`、`019feb4b-21e9-7081-a034-6d251157259f`、
`019feb4b-2237-7062-b5fc-7cd5ae746d2a`、`019feb4b-2293-7c72-a088-2d041f734fd4`；
本轮等待超时后关闭，未返回意见。实现后独立复核 `019feb4e-347f-7ae2-9105-3953373c102a`
等待超时后关闭；effect 所有权修复前再次调用架构角色 `019feb4f-b43b-7910-8413-15e885f53fab`，
同样超时后关闭。父代理完成五轴复核并记录为 bounded audit GO；简化项是抽出共享 motion policy，
并拒绝覆盖已存在 graphics effect。

## 验证证据

```text
check.ps1                 PASS  145 files <= 1000; 3 themes; 22 tokens; 19 selectors; Ruff/compileall
python -m compileall -q src PASS
presentation import       PASS  UI164_PRESENTATION_IMPORT_PASS
package.ps1 onefile       PASS  local-ui-1.64
provenance verify          PASS
root/canonical hash       PASS  equal
```

本轮遵守项目“默认不启动软件”的授权边界，没有启动 GUI/EXE、后台服务或真实设备，
也没有运行 offscreen/HIDPI/读屏/硬件验收；因此实际动画帧差分和 Windows 原生字体仍待用户授权。
嵌入式 C/C++ 适用性为 N/A。

## 最终包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- source revision：`local-ui-1.64`
- size：`47,878,874` bytes
- SHA-256：`58CD1A56BF21ADDA900DCFF333C8D47FE7343E15A6DA567A118FAD4C831D4EDA`
- archive listing SHA-256：`76EEB528D705FA474BF5278D5255D0C51C3573EDCFEE7F028C983D25EBCC6CEF`
- signature：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
