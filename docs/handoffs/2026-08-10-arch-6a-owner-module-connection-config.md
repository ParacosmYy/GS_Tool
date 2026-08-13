# ARCH-6a：连接配置 owner-module contract 迁移

日期：2026-08-10  
范围：MainWindow facade 渐进收窄、连接配置 builder owner 边界、非破坏性组合验证和 onefile 交付  
父代理：Codex；父代理是唯一写入者  
工作区：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建或操作 worktree

## 变更

- 从 `presentation/main_window.py` 移除 8 个纯配置/解析转发：
  `_build_uart_config`、`_build_tcp_config`、`_build_rtt_config`、`_build_udp_config`、
  `_build_server_config`、`_build_ble_discovery_config`、`_build_ble_config`、`_parse_ble_uuid_filter`。
- `controllers/connection_runtime.py` 的连接分支直接调用本模块 `build_*_config()`；BLE config 内部直接调用
  `parse_ble_uuid_filter()`。
- `controllers/ble.py` 显式导入并调用 `build_ble_discovery_config(window)`，不再依赖 MainWindow 的私有 builder。
- 未使用 `__getattr__`、动态注册、monkey-patch 或 mixin；保留 `MainWindow(QMainWindow)`、Qt parent、signals、callbacks、
  objectName、Tab 顺序、主题和生命周期。

## 架构决策

Terra/max 架构师 `019fe925-12ae-7940-8f41-19183bf025d4` 只读评审：

- 全量 typed facade object 或 mixin 一次性替换为 NO-GO，原因是当前 15 个 controller 约有 2102 个 `window._*` 访问，
  一次迁移会扩大风险面。
- 本轮 owner-module contract 切片 GO：先迁移无 Qt signal/callback 的纯配置 builder，再按 workspace/terminal/commands/
  protocol family 逐族建立窄 feature DTO。

独立复核 `019fe92a-b62b-7f13-ad55-1533088b2677` 初次误把合法组合方法当成旧 builder，返回 NO-GO；父代理提供当前 checkout
精确扫描后，复核员基于当前文件更正为 GO，Critical=0、Required=0、Optional=2。该更正结论记录在最新 handoff，不采纳
过时扫描结果。

## 验证

```text
pwsh .\scripts\check.ps1                         pass (109 files <= 1000)
ruff / py_compile                                 pass
ARCH6_CONNECTION_OWNER                            pass (980/1180, 3 tabs, hscroll=0)
old facade exact scan                             pass (8 names = 0)
main_window.py                                    951 lines
```

当前 `src` 中以下 8 个精确名称均为 0：

```text
_build_uart_config
_build_tcp_config
_build_rtt_config
_build_udp_config
_build_server_config
_build_ble_discovery_config
_build_ble_config
_parse_ble_uuid_filter
```

offscreen 环境缺少 PySide6 fonts 目录，中文可能显示方框；未运行持续 GUI、HIDPI、真实读屏、EXE 启动/退出、真实传输、
硬件或正式发行验收。没有创建或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 最新包

- canonical：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)
- SHA-256：`E4227C013FB1658104D9D599699CEE8D4B78B2F351B8F278C48EF0F2721B5E0E`
- 大小：`47,750,999` bytes
- canonical/root hash 与 size 一致；`PROVENANCE.json` verification 通过。
- `signature.status=NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`，属于 engineering build。

## Embedded R&D applicability

本轮只修改 Python/PySide6 presentation 与文档，不涉及 embedded C/C++、MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、驱动、
bootloader、OTA 固件、Flash/NVM、功率或电机控制；厂商公开资料适用性为 N/A，不声称 MISRA/ISO 26262/ASPICE 或任何
认证。按项目门禁完成架构审查、独立复核、行为保持简化评估和授权的非破坏性验证。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
