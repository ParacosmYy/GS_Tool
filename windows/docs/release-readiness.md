# 发布就绪审计（持续更新）

**作者：** AI Token Tracker Engineering Team
**维护者：** Project Owner
**状态：** In Progress；未达到最终交付条件
**最后核对：** 2026-08-10

## 目的

本文件是架构师交付交接物，不是“源码能启动”证明。只有表格中的所有必要项都达到
`pass`，并且真实运行证据与部署负责人记录齐全，才允许把长期目标标记为完成。

## 当前门禁矩阵

| 范围 | 当前状态 | 证据/下一步 |
| --- | --- | --- |
| Windows 模块化单体、CLI、SQLite、Web、RBAC | pass（源码/本机运行） | `compileall`、CLI help、health、API/安全边界和架构文档已核对 |
| 自动 Provider usage / Gateway | 条件通过 | Gateway、SSE usage、Usage Ingest、DPAPI 重试队列已完成；仍需用户本人合法 Provider Key 做一次非流式/流式联调 |
| Web UI-1/UI-2 | pass（源码/本机视觉） | v7 场景、显式主图层、动效和首帧可读性截图已保存 |
| Web UI-3 | in progress | 登录页 320/768/1024/1440 观察已完成；受保护页面合法会话、真实设备指标、焦点/错误/导出/API 失败路径仍待证据 |
| EXE | pending | `packaging/toolchain-doctor.ps1` 当前明确等待 PyInstaller；批准环境需生成、启动、持久化、升级和签名验收 |
| Android APK | pending | `android/toolchain-doctor.ps1` 当前等待 JDK 17、Gradle wrapper 和 Android SDK API 37/build-tools |
| HTTPS 中心部署 | pending | Caddy、正式域名、ACL、证书、日志轮转和外部 health/ready 检查需在部署主机完成 |
| 备份恢复/回滚 | 条件通过 | 隔离 staging 恢复和 schema 校验已完成；真实数据恢复、保留周期和负责人记录仍待部署演练 |
| 工程质量门禁 | pass（当前 checkout） | `token_tracker audit --json`：源码行数、作者头、Python docstring、Android KDoc、契约引用、跨端资产和 Chart.js 供应链通过 |

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
