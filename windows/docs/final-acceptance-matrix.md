# 最终交付验收矩阵

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** In Progress；本文件不是完成签署  
**最后核对：** 2026-08-10

## 使用规则

本矩阵把用户需求、工程约束和部署门禁映射到可复核证据。`pass` 只表示当前范围已有直接证据；
`conditional` 表示源码或隔离环境通过，但仍缺真实环境；`pending` 表示必须等待外部工具、凭据、
部署权限或授权设备。只有所有必要项均为 `pass`，并且架构师签署项完成，才允许关闭长期目标。

## 需求与交付状态

| 编号 | 需求/约束 | 当前状态 | 直接证据 | 剩余门禁 |
| --- | --- | --- | --- | --- |
| R-01 | 记录模型、输入/输出 token、本地时间、备注并持久化 | pass | `schema.py`、`services.py`、SQLite schema、CLI/API smoke | 真实部署数据演练 |
| R-02 | 按日/周/月/全部汇总并计算总量 | pass | `services.py`、`/api/summary`、Dashboard 周期切换证据 | 真实生产数据回归 |
| R-03 | CLI 添加、汇总、导出、服务和 `--help` | pass | `cli.py`、README quick start、CLI help 记录 | 无 |
| R-04 | Web 可视化趋势、模型占比、历史记录 | conditional | Dashboard、Chart.js 本地资产、v12 场景、UI-3 v17/v18 证据 | 真实设备指标、系统动效环境 |
| R-05 | Provider URL/Key 输入、自动模型发现、usage 自动归档 | conditional | `provider_service.py`、adapter registry、Web/Android 接入、Gateway 动态中心发现（ADR-075） | 用户合法 Key 的真实非流式/流式联调 |
| R-06 | 多用户同库、管理员 RBAC、查看成员 token/事件/log | pass | `admin_service.py`、RBAC、Admin UI token/event/log 详情、API v1 | 正式中心主机部署 |
| R-07 | 工作方向、效率、正确/错误码、脱敏日志和导出 | conditional | Web Activity 竖切片（ADR-072、UI-3 v19）、`/api/v1/events/work`、`work_events`/`app_logs` schema、管理员读模型/CSV | 真实部署日志样本和数据保留策略 |
| R-08 | Android 同账号联动 | conditional | Compose 登录、Remote/Repository/DTO/API v1 契约、v12 共享场景、项目内 JDK/Gradle/Wrapper、系统减少动画 UI 边界（ADR-079/086/081） | 用户接受 SDK license 后安装 API 37/Build Tools、APK 构建与授权设备安装联调 |
| R-09 | Windows 个人体验入口可直接启动 | pass | 根目录 `start.bat`、根 `run.py` shim、`windows/run.py` 端口无副作用回退、个人入口 loopback fail-closed（ADR-078） | 重启后用户数据持久化验收 |
| R-10 | EXE 便捷分发 | conditional | PyInstaller 6.22.0 lock、`packaging/build.ps1`/`package.ps1`、ADR-082/087、v12 EXE/ZIP SHA-256、隔离 `/login`/`ready`/资源 200、真实记录重启读取与 `%LOCALAPPDATA%` 建库证据 | 升级、回滚、签名和正式分发 |
| R-11 | 局域网/公网分享且数据集中在管理员电脑 | conditional | `share-doctor`、LAN wrapper、Caddy/Waitress 契约、项目内 Caddy v2.11.4 固定下载/校验/预检/启动入口（ADR-083）；本机缓存、版本、SHA-512 和示例 `validate` 已通过 | 真实域名、证书、防火墙、ACL、日志轮转和外部 health |
| R-12 | 备份、恢复、回滚 | conditional | `backup`/`verify-backup`/`restore-backup`/`backup-inventory`、`release-doctor -CheckBackups`、ADR-060/077、staging 演练 | 真实数据恢复、负责人确认的保留周期/离线副本和回滚记录 |
| R-13 | 前后端边界、API v1、角色分工和模块化架构 | pass | `api-contract.md`、roles、ui-modules、ADR 目录、层间审计 | 架构师最终签署 |
| R-14 | 每个文件不超过 1000 行，企业级头部和注释 | pass | `token_tracker audit --json` 行数/头部/docstring/KDoc 门禁 | 新增代码继续纳入审计 |
| R-15 | 项目内 skills、UI 规范和可维护交付流程 | pass | `windows/skills/`、角色约束、ADR、`release-doctor` | 最终归档与版本签署 |
| R-16 | UI 可读、动画降级、焦点、ARIA、错误态 | conditional | UI-3 v14/v15/v16/v17/v18/v19/v23 运行证据、motion contract、ADR-063 | 真实设备、reduced-motion、高对比度、下载落盘 |
| R-17 | 一条可重复的交付预检命令 | pass | `release-doctor.ps1/.bat`、`ci/quality-gate.ps1`、ADR-071/085，Local/LAN/Production 与源码质量门禁入口 | 远程仓库首次 CI 执行、外部工具准备后重新执行 |

## 当前未关闭的硬门禁

1. 合法 Provider Key 的脱敏非流式与流式 usage 联调。
2. Android SDK API 37/Build Tools、APK 安装和同账号联调；项目内 JDK 17、Gradle Wrapper 9.5.0 已准备但仍需构建证据。
3. EXE 升级/回滚、签名和正式分发验收；v12 构建、启动、真实记录重启读取和默认用户目录证据已记录在 ADR-087，ADR-082 保留历史基线。
4. Caddy/正式域名/证书/ACL/轮转/外部 health 与 ready 验收。
5. 真实数据备份恢复、限流压测、日志样本和保留策略演练。
6. 真实设备的 UI-3 viewport、reduced-motion、高对比度、下载落盘和最终架构师签署。

## 关闭条件

架构师必须逐行检查本矩阵、`release-readiness.md`、`tasks/todo.md` 和所有 ADR；每个 `conditional`/
`pending` 都必须转为有直接证据的 `pass`，工作树必须干净，敏感数据不得进入 Git，之后才可以调用
最终目标完成门禁。
