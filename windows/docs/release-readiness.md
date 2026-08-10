# 发布就绪审计（持续更新）

**作者：** AI Token Tracker Engineering Team
**维护者：** Project Owner
**状态：** In Progress；未达到最终交付条件
**最后核对：** 2026-08-10

## 目的

本文件是架构师交付交接物，不是“源码能启动”证明。只有表格中的所有必要项都达到
`pass`，并且真实运行证据与部署负责人记录齐全，才允许把长期目标标记为完成。

逐条需求、证据和剩余门禁的唯一核对表见 [`final-acceptance-matrix.md`](final-acceptance-matrix.md)。

## 当前门禁矩阵

| 范围 | 当前状态 | 证据/下一步 |
| --- | --- | --- |
| Windows 模块化单体、CLI、SQLite、Web、RBAC | pass（源码/本机运行） | `compileall`、CLI help、health、根 `run.py` shim、Waitress 本机 WSGI、个人入口 loopback fail-closed、API/安全边界和架构文档已核对（ADR-078） |
| 自动 Provider usage / Gateway | 条件通过 | Gateway、SSE usage、Usage Ingest、DPAPI 重试队列、运行时队列故障 fail-closed、动态中心端口只读发现、响应读取/幂等键/非回环 HTTPS 门禁已完成（ADR-058/059/068/075）；仍需用户本人合法 Provider Key 做一次非流式/流式联调 |
| Web UI-1/UI-2 | pass（源码/本机视觉） | v10 场景、ADR-074 可见度基线、ADR-076 场景资产决策、显式主图层、动效和首帧可读性已在临时源码实例复核 |
| Web UI-3 | in progress | 登录页四档观察、认证页 v23 唯一语义 h1/aria-hidden glitch 层、隔离合法会话下 Dashboard/Admin 空态、成员脱敏日志详情、动态错误 alert/status 播报、四档 viewport/焦点回流、历史非空、服务端 CSV 200、Provider 502 错误态和个人 Activity 写入切片已有源码/隔离证据；真实设备指标、浏览器下载落盘、真实 Provider 成功、reduced-motion/高对比度环境仍待证据 |
| EXE | pending | `packaging/toolchain-doctor.ps1` 当前明确等待 PyInstaller；批准环境需生成、启动、持久化、升级和签名验收 |
| Android APK | pending | 版本契约已固定为 AGP 9.3.0 / Gradle 9.5.0 / JDK 17 / API 37 / Compose BOM 2026.06.00；`android/toolchain-doctor.ps1` 仍等待实际 JDK、Wrapper、SDK 和 APK 验收 |
| HTTPS 中心部署 | pending | 应用生产绑定已强制 loopback（ADR-061）；Caddy、正式域名、ACL、证书、日志轮转和外部 health/ready 检查仍需在部署主机完成 |
| 备份恢复/回滚 | 条件通过 | 隔离 staging 恢复、当前 schema（含 Usage Ingest Token）校验、防覆盖命名、只读 `backup-inventory --verify` 和 `release-doctor -CheckBackups` 接入已完成（ADR-060/077）；真实数据恢复、负责人确认的保留周期/离线副本和回滚记录仍待部署演练 |
| 工程质量门禁 | pass（当前 checkout） | `token_tracker audit --json`：代码/文本 1000 行门禁、作者头、Python docstring、Android KDoc、Web UI 静态契约、production/Caddy/LAN 部署契约、层间依赖边界、契约引用、跨端资产和 Chart.js 供应链通过；`release-doctor` 提供统一只读编排入口 |

## 每次交付必须执行

从 `windows/` 目录执行：

```powershell
.\.venv\Scripts\python.exe -m compileall -q token_tracker
.\.venv\Scripts\python.exe -m token_tracker audit --json
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\packaging\toolchain-doctor.ps1
```

Android 和正式边缘门禁只能在对应环境执行；本地缺工具时保留 `pending`，不下载、不伪造
APK/EXE/Caddy 通过证据。所有变更必须回写 `tasks/todo.md`、必要 ADR 和本文件。

## 最终签署条件

1. Android APK 已由批准工具链构建、安装到授权设备并完成同账号联调。
2. EXE 已生成并验证用户数据目录、重启持久化、升级和回滚。
3. 真实 Provider usage 已完成脱敏非流式/流式证据；Key 未进入仓库、日志或数据库。
4. 合法认证会话下的 Web 五模块和管理员路径完成四档响应式、ARIA、焦点、错误态和导出证据。
5. HTTPS、备份恢复、限流、脱敏日志、ACL、轮转和回滚由部署负责人记录并复核。
6. 架构师检查依赖方向、契约、ADR、注释、行数和 Git 交付状态后，才可签署完成。
