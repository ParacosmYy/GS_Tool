# 最终交付验收矩阵

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**状态：** In Progress；本文件不是完成签署  
**最后核对：** 2026-08-11

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
| R-04 | Web 可视化趋势、模型占比、历史记录 | conditional | Dashboard、Chart.js 本地资产、v14 场景、UI-3 v17/v18 证据 | 真实设备指标、系统动效环境 |
| R-05 | Provider URL/Key 输入、自动模型发现、usage 自动归档 | conditional | `provider_service.py`、adapter registry、Web/Android 接入、Gateway 动态中心发现（ADR-075） | 用户合法 Key 的真实非流式/流式联调 |
| R-06 | 多用户同库、管理员 RBAC、查看成员 token/事件/log | pass | `admin_service.py`、RBAC、Admin UI token/event/log 详情、API v1 | 正式中心主机部署 |
| R-07 | 工作方向、效率、正确/错误码、脱敏日志和导出 | conditional | Web Activity 竖切片（ADR-072、UI-3 v19）、`/api/v1/events/work`、`work_events`/`app_logs` schema、管理员读模型/CSV | 真实部署日志样本和数据保留策略 |
| R-08 | Android 同账号联动 | conditional | Compose 登录、Remote/Repository/DTO/API v1 契约、v14 共享场景、项目内 JDK/Gradle/Wrapper、用户授权 SDK 入口、系统减少动画 UI 边界（ADR-079/095/081/098） | 用户接受 SDK license 后安装 API 37/Build Tools、APK 构建与授权设备安装联调 |
| R-09 | Windows 个人体验入口可直接启动 | pass | 根目录 `start.bat`、根 `run.py` shim、`windows/run.py` 端口无副作用回退、个人入口 loopback fail-closed（ADR-078） | 重启后用户数据持久化验收 |
| R-10 | EXE 便捷分发 | conditional | PyInstaller 6.22.0 lock、`packaging/build.ps1`/`package.ps1`/`sign-build.ps1`/`verify-signature.ps1`/`verify-upgrade-rollback.ps1`、ADR-082/087/089/090/091/092/093/095/096、v14 EXE/ZIP SHA-256、包 manifest/verifier、隔离 `/login`/`ready`/v14 资源 200、真实记录重启读取和 v13→v14→v13 数据保留证据 | 批准的 Authenticode 证书/SignTool/时间戳服务和正式分发 |
| R-11 | 局域网/公网分享且数据集中在管理员电脑 | conditional | `share-doctor`、LAN wrapper、Caddy/Waitress 契约、项目内 Caddy v2.11.4 固定下载/校验/预检/启动入口（ADR-083）；本机缓存、版本、SHA-512、示例 `validate`、2026-08-10 LAN handoff 与示例 Production edge preflight 已通过（ADR-097） | 真实域名、证书、防火墙、ACL、日志轮转和外部 health |
| R-12 | 备份、恢复、回滚 | conditional | `backup`/`verify-backup`/`restore-backup`/`backup-inventory`、`release-doctor -CheckBackups`、已填充隔离数据库备份/恢复证据（ADR-060/077/088） | 真实中心数据恢复、负责人确认的保留周期/离线副本和回滚记录 |
| R-13 | 前后端边界、API v1、角色分工和模块化架构 | pass | `api-contract.md`、roles、ui-modules、ADR 目录、层间审计 | 架构师最终签署 |
| R-14 | 每个文件不超过 1000 行，企业级头部和注释 | pass | `token_tracker audit --json` 行数/头部/docstring/KDoc 门禁 | 新增代码继续纳入审计 |
| R-15 | 项目内 skills、UI 规范和可维护交付流程 | pass | `windows/skills/`、角色约束、ADR、`release-doctor` | 最终归档与版本签署 |
| R-16 | UI 可读、动画降级、焦点、ARIA、错误态 | conditional | UI-3 v14/v15/v16/v17/v18/v19/v23/v26/v27/v29/v30/v31/v32/v33/v34/v35/v36/v37/v38/v39/v40/v41/v42/v45/v46/v47/v48/v49 运行证据、motion contract、ADR-063；v27 补齐当前 v14 登录页四档结构，v29 补齐场景遮罩/导航玻璃层，v30 补齐 Dashboard 空态行动入口、非空图表回归和四档运行值，v31 补齐登录/Admin 四档首帧、无溢出、Admin 详情焦点回流和清洁控制台，v32 补齐 Admin 共享 pointer/backdrop/reveal/surface motion、动态详情可读性、焦点回流与清洁控制台观察，v33 补齐导航 active/aria-current、滚动进度线、顶部半透明玻璃层和桌面/移动端复核，v34 补齐自动采集双栏说明卡片收口、320px 单列与清洁控制台观察，v35 补齐待显区块基础可读性、锚点动画收束和 320px 无溢出，v36 补齐 Dashboard/Activity/Admin 表格零信号空态、微动效降级与移动端复核，v37 补齐四周期 active pill、aria-pressed 和 320px 几何同步，v38 补齐暗色 select、箭头、option 可读性与 320px 交互复核，v39 补齐顶部 AI TOKEN 品牌栏透景、滚动保持和移动端无溢出复核，v40 补齐异常补录 disclosure 的 open/closed 状态、Space/Enter 键盘交互与 320px 无溢出复核，v41 补齐主要章节的 sticky header 安全落点和移动端无溢出复核，v42 补齐图表/工作事件/Token 历史数据面阅读遮罩、空/非空状态和 320px 无溢出复核，v45 补齐滚动态透明羽化品牌栏、移除背景 blur、桌面/320px 无溢出和本地控制台复核，v46 补齐自动采集/补录深链接的 sticky header 安全落点与桌面/320px 收束复核，v47 补齐基础态品牌栏透明度、移动/桌面截图和滚动态复核，v48 补齐登录/注册半透明表面、四档响应式和可读性复核，v49 补齐 pointer follower 交互态尺度、surface spotlight、按钮磁吸与离开态复核 | 真实设备、reduced-motion、高对比度、下载落盘 |
| R-17 | 一条可重复的交付预检命令 | pass | `release-doctor.ps1/.bat`、`ci/quality-gate.ps1`、ADR-071/085，Local/LAN/Production 与源码质量门禁入口 | 远程仓库首次 CI 执行、外部工具准备后重新执行 |

### v43 交付增量

- Token 历史桌面表格强化模型、总 token 和来源层级；320px 转为完整信息卡，空态保持原有语义与信标。
- 顶部 `AI TOKEN` 品牌栏改为透明背景，仅保留轻量 blur、底线与文字阴影；本增量不改变导航语义或动效契约。
- 直接证据与限制见 [`ui-accessibility-evidence.md`](ui-accessibility-evidence.md) 的 v43 章节；真实设备、reduced-motion、
  高对比度和浏览器下载落盘仍属于未关闭门禁。

### v44 交付增量

- sticky header 首屏透明、滚动后自适应玻璃态，避免 hero 大字透景造成导航噪声；登录页与 Dashboard 共用同一滚动上下文。
- 通过隔离浏览器确认桌面/320px、空登录态、透明/滚动态 computed style、本地控制台与横向溢出；限制见 v44 UI 证据。

### v45 交付增量

- 滚动态 AI TOKEN 品牌栏改为透明羽化渐变并关闭滚动态背景 blur，保留首屏透明态、文字层级和既有滚动状态调度。
- 通过隔离浏览器确认桌面/320px 首屏与滚动态 computed style、品牌栏截图、横向溢出和本地控制台；限制见 v45 UI 证据。

### v46 交付增量

- 自动采集 CTA 与补录深链接补充统一安全滚动落点，避免 sticky header 遮住目标卡片标题。
- 通过隔离浏览器确认桌面/320px 平滑滚动收束、header 几何、无横向溢出和本地控制台；限制见 v46 UI 证据。

### v47 交付增量

- 移除 AI TOKEN 品牌栏基础态的背景 blur 与外部阴影，保留透明透景、细边线和文字可读性。
- 通过隔离浏览器确认桌面/320px 首屏、滚动态 computed style、截图、横向溢出和本地控制台；限制见 v47 UI 证据。

### v48 交付增量

- 登录/注册共享卡片降低背景遮罩和 blur 强度，让角色场景真实透过半透明表面，同时保留高对比度兜底。
- 通过隔离浏览器确认 320/768/1024/1440 四档、注册页复用、截图、横向溢出和本地控制台；限制见 v48 UI 证据。

### v49 交付增量

- 收敛交互态 pointer aura/ring 的尺寸和亮度，保留跟随、surface spotlight 与按钮磁吸反馈，不压过原生焦点边界。
- 通过隔离浏览器确认 pointer-ready/interactive、surface 变量、按钮磁吸、离开态回收和本地控制台；限制见 v49 UI 证据。

### v50 交付增量

- 滚动时为 AI TOKEN 品牌栏增加半透明渐隐层与轻微 blur；首屏仍保持完全透明，解决滚动态统计文案穿过顶栏的问题。
- 通过隔离 Dashboard 确认桌面首屏/滚动态截图、computed style、skip-link 焦点、无横向溢出和本地控制台；移动矩阵沿用既有证据，限制见 v50 UI 证据。

### v51 交付增量

- 深链接目标 section 清除额外的文档顺序 reveal 延迟，使用 `.56s` 收束，让平滑滚动和内容出现保持同一节奏。
- 通过隔离 Dashboard 确认 Activity/Analysis 目标 computed style、sticky header 落点、截图、无横向溢出、skip-link 焦点和本地控制台；移动矩阵沿用既有证据，限制见 v51 UI 证据。

### v52 交付增量

- 320px hero CTA 改为两列等宽布局，主采集和分析入口在首屏同时可见；768px/默认桌面保持原 flex 布局。
- 通过隔离浏览器确认 320/768/默认桌面 CTA 几何、无横向溢出、skip-link 焦点和本地控制台；1024/1440 与真实设备限制见 v52 UI 证据。

### v53 交付增量

- AI TOKEN 品牌栏首屏与滚动态统一使用透明背景，移除滚动态黑色渐隐层与 blur，保留边线和进度线作为最小层次提示。
- 通过隔离浏览器确认 `scrollY=0/1430` 的 computed style、滚动截图、`background=rgba(0,0,0,0)`、`backdrop-filter=none` 和清洁控制台；完整响应式、设备、辅助偏好及正式部署限制见 v53 UI 证据。

### v54 交付增量

- 621–900px 平板 hero 轨道收敛为 `min(46vw,360px)` 并压缩垂直间距，主 CTA 在 768/900px 首屏完整可见；320px 两列 CTA、1024/1440 桌面节奏保持。
- 真实 320/768/900/1024/1440 视口确认按钮坐标、无横向溢出、skip-link 焦点和清洁控制台；真实设备、reduced-motion、forced-colors 与高对比度限制见 v54 UI 证据。

### v55 交付增量

- AI TOKEN 顶部品牌栏使用低 alpha 渐变玻璃膜，初始/滚动态保留背景透景并以轻量 blur、边线和阴影提供可读层次；不改变导航语义或滚动进度线。
- 390px 手机登录/注册表单优先于介绍区，390/768/1024/1440 的主提交按钮均在首屏内，四档页面无横向溢出；真实认证与滚动态顶栏证据见 v55 UI 章节。

### v56 交付增量

- 320/390px Dashboard hero 轨道收敛为 `min(80vw,340px)`，并压缩移动文案间距，CTA 从首屏底边前移；768/1024/1440 的 hero 几何保持。
- 真实 320/390/768/1024/1440 视口确认轨道、CTA、截图、无横向溢出和清洁控制台；真实设备、reduced-motion、forced-colors 与高对比度限制见 v56 UI 证据。

### v57 交付增量

- Dashboard 状态文案与 TOTAL SIGNAL 信标使用低 alpha 透景玻璃层、文字阴影、信号点和窄屏单行约束，错误状态与系统强制配色保持可读。
- 真实 320/390/768/1024/1440 视口确认信号层、总量信标、CTA 首屏几何、截图、无横向溢出和清洁控制台；真实设备、reduced-motion、forced-colors 与高对比度限制见 v57 UI 证据。

### v58 交付增量

- 自动采集连接徽标统一为低 alpha 玻璃状态层，支持未连接、ready、error 和 forced-colors 视觉边界；不改变 provider 状态契约。
- 无缓存真实 390/768/1024/1440 内容区确认徽标几何、自动采集/指南面板无横向溢出和清洁控制台；真实 Provider、设备与辅助偏好限制见 v58 UI 证据。

### v59 交付增量

- 活动轨迹空态按内容语义脱离 `540px` 数据表格最小宽度，避免 390px 手机和 1024px 双列卡片出现横向滚动；非空事件表格行为保持。
- 无缓存真实 390/768/1024/1440 确认空态表格与页面无横向溢出、提示语完整可读、截图和清洁页面控制台；真实非空事件、设备、辅助偏好与 Provider 限制见 v59 UI 证据。

### v60 交付增量

- 分析图表 empty/unavailable 状态改为紧凑 signal checkpoint，减少新用户首屏后的黑色空画布；ready 图表尺寸和交互保持。
- 真实 320/390/768/1024/1440 确认空态图表文案、CTA 和页面无横向溢出，并保存截图/控制台证据；真实非空图表、设备、辅助偏好与 Provider 限制见 v60 UI 证据。

### v61 交付增量

- AI TOKEN 顶部品牌栏改为透明优先：首屏完全透景，滚动态仅使用轻量局部 veil、4px blur、细边线和低强度阴影，避免深色整条横带覆盖背景场景。
- 真实 320/390/768/1024/1440 视口确认首屏透明 computed style、滚动态轻玻璃、截图、无横向溢出和清洁控制台；真实设备、辅助偏好、Provider 联调和正式部署限制见 v61 UI 证据。

### v62 交付增量

- 短高度桌面（`901px+` 宽、`860px-` 高）仅回收 hero stage 底部留白，使 Today/Week/Month/All time 周期切换器完整进入首屏；标题、轨道、CTA 和移动/平板构图保持。
- 真实 1683×845、320/390/768/1024/1440 视口确认周期切换器几何、截图、无横向溢出和清洁控制台；真实设备、辅助偏好、Provider 联调和正式部署限制见 v62 UI 证据。

### v63 交付增量

- Dashboard 场景插画加入高优先级预加载、`fetchpriority=high` 和 `#071321` 深蓝 fallback，避免首帧解码间隙出现纯黑视觉断层；不改变业务数据和交互。
- 冷 origin 真实 768×900 首帧以及 320/390/768/1024/1440 矩阵确认场景完整、图片完成、无横向溢出和清洁控制台；真实设备、辅助偏好、Provider 联调和正式部署限制见 v63 UI 证据。

### v64 交付增量

- 透明顶栏的 AI TOKEN、导航和用户标签由宽泛 `12px` text-shadow 收敛为紧 keyline + 短 halo，保持背景透景并提升滚动状态字形清晰度。
- 真实 1440×900 首屏/Activity 滚动态、390px 截图与 320/390/768/1024/1440 computed matrix 确认文字 opacity、透明 header、无横向溢出和清洁控制台；真实设备、辅助偏好、Provider 联调和正式部署限制见 v64 UI 证据。

### v65 交付增量

- 同文档锚点由原生长距离 smooth scroll 改为 420–760ms 可取消受控过渡，完成时重新校准目标落点，并保留 hash、ARIA active、键盘、触摸和 reduced-motion 语义。
- 真实 1440px Activity、390px auto-entry 及 320/390/768/1024/1440 矩阵确认落点、无横向溢出和清洁控制台；真实 reduced-motion、设备、Provider 联调和正式部署限制见 v65 UI 证据。

### v66 交付增量

- 滚动态 AI TOKEN 顶栏移除残留 `.045` 黑色玻璃和 `blur(4px)`，首屏/滚动均保持 `background=transparent`、`backdrop-filter=none`，只保留发丝边界和进度线。
- 真实 390px/1440px 滚动态 computed style、截图与 320/390/768/1024/1440 矩阵确认透景、文字可扫描和无横向溢出；真实辅助偏好、设备、Provider 联调和正式部署限制见 v66 UI 证据。

### v67 交付增量

- 基础 `style.css` 直接声明顶栏 transparent/无 blur，避免首帧先显示黑玻璃再由后续 CSS 覆盖；不改变滚动态边界、进度线或强制配色规则。
- 真实 390px/1440px 首屏与滚动态、截图及 320/390/768/1024/1440 矩阵确认透景、无横向溢出和清洁页面日志；真实辅助偏好、设备、Provider 联调和正式部署限制见 v67 UI 证据。

### v68 交付增量

- 认证页自动聚焦由强荧光 halo 改为 quiet edge，首次键盘/鼠标操作后恢复完整 `:focus-visible` 反馈；移动端不自动抢焦点。
- 真实桌面首屏、Tab 键焦点、390px 截图和 320/390/768/1024/1440 矩阵确认焦点层级、可访问反馈、无横向溢出和清洁页面日志；真实辅助偏好、设备、Provider 联调和正式部署限制见 v68 UI 证据。

### v69 交付增量

- AI TOKEN 顶栏默认态与滚动态移除背景图、边线、内侧高光和阴影，保持场景直接透景；滚动进度线和文字阴影继续保留层级反馈。
- 5146 隔离实例真实确认 1440px 首屏、PageDown 滚动态、移动首屏及 320/390/768/1024/1440 矩阵均透明且无横向溢出；窄视口不自动抢焦点，真实设备、辅助偏好、Provider 联调和正式部署限制见 v69 UI 证据。

### v70 交付增量

- 移动端静止首屏保持透明顶栏；滚动态仅增加低 alpha 安全渐变、短 blur 与轻阴影，避免首屏 CTA 与 AI TOKEN/Log out 重叠；forced-colors 复位已落在移动断点规则中。
- 5148 隔离实例真实确认 390px `scrollY≈700` 的 CTA/顶栏几何与 1440px 透明滚动态；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署限制见 v70 UI 证据。

### v71 交付增量

- 首屏继续透明透景；滚动态为桌面提供深蓝低透明度安全层，移动端保留较轻安全层，避免指标标题穿入 sticky 导航并保持文字可读性。
- 5150 隔离实例真实确认 1440/390/320px 首屏与滚动态 computed style、截图、无横向溢出和清洁控制台；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署限制见 v71 UI 证据。

### v72 交付增量

- 透明顶栏的非活动导航与用户名提升至 `--ink-soft`，配合紧凑深色 keyline 提升亮色插画上的可读性；首屏不增加实心背景，滚动态安全层保持原契约。
- 5160 隔离实例真实确认 1440/390/768/1024/1440 矩阵、390px 滚动态、可访问性树、截图、无横向溢出和清洁控制台；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署限制见 v72 UI 证据。

### v73 交付增量

- 透明指标栏的关键字形增加局部 keyline，辅助说明提升至 `--ink-soft`；不添加面板背景，不改变透明场景构图或图表逻辑。
- 5170 隔离实例真实确认 390/1440px 截图、滚动态状态、页面宽度与清洁控制台；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署限制见 v73 UI 证据。

### v74 交付增量

- `data-reveal` 在允许动效时以 `.62` 可读性地板、`8px` 位移和 `.72s` 过渡进入视口，落位仍回到 `1 / 0`；reduced-motion 继续完全静止。
- 5170 隔离实例真实确认 1440px 滚动过渡、320/390/768/1024/1440 无横向溢出、无障碍关键语义与清洁控制台；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署限制见 v74 UI 证据。

### v75 交付增量

- 普通模式的 AI TOKEN 顶栏在首屏和滚动态统一保持透明、无背景图、无 blur、无阴影和无底边线；滚动进度线和文字 keyline 继续提供状态与可读性反馈，forced-colors 仍使用系统 Canvas。
- 5171 隔离实例真实确认 1440/390/320/768/1024px 顶栏 computed style、无横向溢出、无障碍关键语义与清洁控制台；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署限制见 v75 UI 证据。

## 当前未关闭的硬门禁

1. 合法 Provider Key 的脱敏非流式与流式 usage 联调。
2. Android SDK API 37/Build Tools、APK 安装和同账号联调；项目内 JDK 17、Gradle Wrapper 9.5.0 已准备但仍需构建证据。
3. 批准的 EXE Authenticode 签名、时间戳验证和正式分发验收；签名入口边界见 ADR-093，v12→v13→v12 升级/回滚、真实记录读取和用户目录保留已记录在 ADR-092。
4. Caddy/正式域名/证书/ACL/轮转/外部 health 与 ready 验收。
5. 真实数据备份恢复、限流压测、日志样本和保留策略演练。
6. 真实设备的 UI-3 viewport、reduced-motion、高对比度、下载落盘和最终架构师签署。

## 关闭条件

架构师必须逐行检查本矩阵、`release-readiness.md`、`tasks/todo.md` 和所有 ADR；每个 `conditional`/
`pending` 都必须转为有直接证据的 `pass`，工作树必须干净，敏感数据不得进入 Git，之后才可以调用
最终目标完成门禁。
