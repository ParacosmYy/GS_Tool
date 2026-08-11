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
| Windows 模块化单体、CLI、SQLite、Web、RBAC | pass（源码/本机运行） | `compileall`、CLI help、health、根 `run.py` shim、CLI/根入口 Waitress 本机 WSGI、个人入口与 CLI loopback fail-closed、API/安全边界和架构文档已核对（ADR-078/094） |
| 自动 Provider usage / Gateway | 条件通过 | Gateway、SSE usage、Usage Ingest、DPAPI 重试队列、运行时队列故障 fail-closed、动态中心端口只读发现、响应读取/幂等键/非回环 HTTPS 门禁已完成（ADR-058/059/068/075）；仍需用户本人合法 Provider Key 做一次非流式/流式联调 |
| Web UI-1/UI-2 | pass（源码/本机视觉） | v14 场景、v29 左上遮罩/导航玻璃层、v31 登录首帧与左上环境光运行观察、v32 Admin 共享 pointer/backdrop/reveal/surface motion 与动态详情可读性、v33 顶部半透明玻璃导航与滚动章节状态、v34 自动采集说明卡片表面收口、v35 待显区块可读性和锚点动画收束、v36 表格零信号空态信标与扫描线、v37 周期切换 active pill 与 pressed 状态同步、v38 select 暗色玻璃表面与箭头、v39 顶部 AI TOKEN 品牌栏透景遮罩收敛与滚动保持、v40 异常补录 disclosure 状态层次与展开过渡、桌面/320px 复核、ADR-074 可见度基线、ADR-095 场景资产决策、显式主图层、动效和首帧可读性已在临时源码实例复核；v13/v12/v11/v10/v9/v8 保留回滚 |
| Web UI-3 | in progress | 登录页四档观察（v27/v29/v31）、Dashboard v30 空态行动入口/非空图表回归、Admin v31 数据轨道/详情焦点回流、v32 动态详情打开后的可读性、焦点回流和清洁控制台、v33 导航 active/aria-current、滚动进度线、顶部半透明玻璃层和 320px 无溢出、v34 自动采集双栏收口、320px 单列和清洁控制台、v35 待显区块可读性、锚点进入视口后的动画收束和 320px 无溢出、v36 Dashboard/Activity/Admin 表格零信号空态、微动效和 320px 无溢出、v37 四周期 active pill、aria-pressed、锚点/几何同步和 320px 无溢出、v38 暗色 select、焦点样式、实际选项切换和 320px 无溢出、v39 顶部品牌栏透明层、滚动保持和 320px 无溢出、v40 异常补录 disclosure 的 Space/Enter 交互、焦点和 320px 无溢出、认证页 v23 唯一语义 h1/aria-hidden glitch 层、隔离合法会话下 Dashboard/Admin 空态、成员脱敏日志详情、动态错误 alert/status 播报、历史非空、服务端 CSV 200、Provider 502 错误态、个人 Activity 写入切片和 v14 默认浏览器首屏均已有源码/隔离证据；真实设备指标、浏览器下载落盘、真实 Provider 成功、reduced-motion/高对比度环境仍待证据 |
| EXE | conditional | PyInstaller 6.22.0 已锁定并按 v14 资源重建 onedir EXE/ZIP；v14 包 verifier、`/login`、`/api/v1/ready`、v14 资源 200、隔离 `%LOCALAPPDATA%` 建库和 EXE 目录无 data 已通过；v13→v14→v13 真实 API 升级/回滚与记录保留已通过（ADR-091/092/096）；签名入口/验证器已 fail-closed 接入（ADR-093），实际 Authenticode 和正式分发仍待验收 |
| Android APK | pending | 项目内 JDK 17、Gradle 9.5.0、command-line tools 和官方 Wrapper 已准备并由 doctor 识别；`accept-sdk-license.bat` 已提供用户授权入口；Android v14 背景与 SignalOrbit 已接入系统减少动画边界（ADR-079/095），API 37/Build Tools 因 SDK license 尚未交互确认仍 pending，APK/设备联调未验收（ADR-081/098） |
| HTTPS 中心部署 | pending | 应用生产绑定已强制 loopback（ADR-061）；Caddy v2.11.4 已在项目缓存中完成 SHA-512、版本和示例配置 validate，配置预检与显式 edge 启动器已接入（ADR-083）；2026-08-10 的 LAN handoff 与示例 Production edge preflight 均通过，但正式域名、ACL、证书、日志轮转和外部 health/ready 检查仍需在部署主机完成（ADR-097） |
| 备份恢复/回滚 | 条件通过 | 已填充隔离 EXE 数据库完成备份、`verify-backup`、`backup-inventory --verify` 和 staging 恢复二次校验，当前 schema（含 Usage Ingest Token）与防覆盖命名仍受门禁保护（ADR-060/077/088）；真实数据恢复、负责人确认的保留周期/离线副本和回滚记录仍待部署演练 |
| 工程质量门禁 | pass（当前 checkout） | `token_tracker audit --json`：代码/文本 1000 行门禁、作者头、Python docstring、Android KDoc、Web UI 静态契约、production/Caddy/LAN 部署契约、层间依赖边界、契约引用、跨端资产和 Chart.js 供应链通过；`windows/ci/quality-gate.ps1` 与 GitHub Actions 已接入（ADR-085）；`release-doctor` 提供统一只读编排入口 |

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
