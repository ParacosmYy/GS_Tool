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

### v76 交付增量

- Dashboard 内容面板在普通配色下改为深蓝玻璃层：`.84/.92` alpha、`16px` blur 和原有边框/文字 token 共同维持可读性；AI TOKEN 顶栏透明契约不变。
- 5172 隔离实例真实确认 390/320/768/1024/1440px 面板 computed style、无横向溢出、无障碍关键语义与清洁控制台；`forced-colors` 媒体边界已落地，真实设备、辅助偏好、Provider 联调和正式部署限制见 v76 UI 证据。

### v77 交付增量

- 场景 veil 在桌面、平板和移动断点改为低 alpha 深蓝氛围层；AI TOKEN 顶栏继续保持 transparent、无 blur、无阴影和无底边线，避免顶部视觉变成黑色实心条。
- 5173 隔离实例真实确认 390/320/768/1024/1440px 顶栏 computed style、无横向溢出、无障碍关键语义与清洁控制台；独立证据见 `ui-accessibility-evidence-v77.md`，真实设备与辅助偏好继续独立门禁。

### v78 交付增量

- 登录页自动聚焦从荧光绿报警态改为低亮 lavender keyline；marker 清除后，Tab 键路径仍显示完整 lime `:focus-visible` 环，不削弱键盘可达性。
- 5174 隔离实例真实确认 320/390/768/1024/1440px computed style、桌面/移动截图、Tab 焦点、无横向溢出、无障碍关键语义与清洁控制台；独立证据见 `ui-accessibility-evidence-v78.md`，真实设备与辅助偏好继续独立门禁。

### v79 交付增量

- Dashboard AI TOKEN 品牌区保持透明；滚动态为右侧 nav/account cluster 增加低 alpha 局部 reading rail，pointer-events 保持 none，避免导航与 CTA/表单内容重叠。
- 5175 隔离实例真实确认 320/390/768/1024/1440px 顶部矩阵、768/1440px 滚动态截图和 computed style、无横向溢出、无障碍关键语义与清洁控制台；独立证据见 `ui-accessibility-evidence-v79.md`，真实设备与辅助偏好继续独立门禁。

### v80 交付增量

- 零数据图表增加低对比网格、零基线、等待扫描线与模型占比导引环；不新增 DOM，不改变 Chart.js 数据契约，reduced-motion/forced-colors 保留静止与系统配色边界。
- 5176 隔离实例真实确认 320/390/768/1024/1440px 图表尺寸与无横向溢出、Week/Today 周期交互、无障碍关键语义和清洁页面控制台；独立证据见 `ui-accessibility-evidence-v80.md`，真实设备与辅助偏好继续独立门禁。

### v81 交付增量

- 滚动态 AI TOKEN 品牌锁定区增加低 alpha 局部透景 lens，`Scroll to explore` cue 在滚动后离开 sticky header 阅读通道；整条顶栏仍为 transparent/no global blur，不新增 DOM 或焦点 stop。
- 5177 隔离实例真实确认 320/390/768/1024/1440px 顶部矩阵、390/1440px 滚动态截图与 computed style、Week/Today 周期交互、无障碍关键语义和清洁页面控制台；独立证据见 `ui-accessibility-evidence-v81.md`，真实设备与辅助偏好继续独立门禁。

### v82 交付增量

- 装饰性 pointer follower 增加显式生命周期：首帧 hidden、fine-pointer 移动时 visible、1.8 秒静止后 `pointer-idle` hidden 并取消动画帧、再次移动恢复；不改变 DOM 节点、焦点、hover/focus 语义或业务数据。
- 5178 隔离实例真实确认指针状态序列、无横向溢出、Week/Today 周期交互、无障碍关键语义和清洁页面控制台；独立证据见 `ui-accessibility-evidence-v82.md`，四档 viewport、真实设备与辅助偏好继续独立门禁。

### v83 交付增量

- 滚动态 AI TOKEN 顶栏移除右侧 reading rail 与品牌 lockup lens；普通模式继续 `transparent / no blur / no shadow`，文字 keyline 和进度线保持不变。
- 5179 隔离实例真实确认默认桌面首屏与 `scrollY = 720` 截图、computed style、Week/Today 周期交互、无横向溢出、可访问性树和清洁页面日志；新证据见 `ui-accessibility-evidence-v83.md`，viewport override、真实设备与辅助偏好继续独立门禁。

### v84 交付增量

- `TOTAL SIGNAL` 计数器增加现有 DOM 内的 `::before` 扫描弧，使用 transform-only `core-sweep` 动画；forced-colors 隐藏，reduced-motion 静止，不改变数据与可访问性结构。
- 5181 隔离实例真实确认默认桌面首屏、动画时间采样、无横向溢出、可访问性语义和清洁页面日志；新证据见 `ui-accessibility-evidence-v84.md`，周期 pill 同步与 viewport override、真实设备和辅助偏好继续独立门禁。

### v85 交付增量

- 周期 active pill 在 pressed state 变更任务内立即同步几何，再通过下一帧完成布局校准；不改变 period API、摘要请求或 ARIA pressed 契约。
- 5181 隔离实例真实确认 Today/Week/Month/All time 四档 active、位置、宽度、状态文案、可访问性和清洁页面日志；新证据见 `ui-accessibility-evidence-v85.md`，viewport override、真实设备与辅助偏好继续独立门禁。

### v86 交付增量

- 滚动态 AI TOKEN 顶栏保持透明填充、无背景图、无边线和无阴影，仅增加无填充的 `blur(10px) saturate(1.04)` 内容软化层；首屏继续 `backdrop-filter=none`，不改变 DOM、导航或业务数据。
- 5182 隔离实例真实确认 `scrollY=0/720/900/1118` 的透明计算值、cue 退出、标题安全落点、截图、可访问性语义和清洁页面日志；新证据见 `ui-accessibility-evidence-v86.md`，viewport override、真实设备与辅助偏好继续独立门禁。

### v87 交付增量

- 滚动态 AI TOKEN 品牌区恢复 `background=transparent`、`backdrop-filter=none`；右侧 nav/account 使用无填充局部镜片，hero actions 在非焦点状态下退出 sticky 阅读区，不改变 DOM、API、认证或业务数据。
- 5183 隔离实例真实确认 `scrollY=0/720/2220`、返回顶部恢复、局部镜片、CTA/cue 状态、焦点控件标签、无横向溢出和清洁页面日志；新证据见 `ui-accessibility-evidence-v87.md`，深度内容自动避让、viewport override、真实设备与辅助偏好继续独立门禁。

### v88 交付增量

- 新增 sticky 内容避让表现模块：穿过透明 AI TOKEN 顶栏的非焦点 `.site-main .button` / `summary` 视觉收束，布局与 Tab 顺序保留；焦点控件通过 `:focus-within` 恢复，不改变 DOM、API、认证或业务数据。
- 5184 隔离实例真实确认 `scrollY≈0/720/2220`、`#proxy-submit` 的避让类/计算样式、Tab 焦点恢复、无横向溢出和清洁页面日志；新证据见 `ui-accessibility-evidence-v88.md`，viewport override、真实设备与辅助偏好继续独立门禁。

### v89 交付增量

- 允许动效下 `[data-reveal]` 的预落位提高至 `.78` opacity、`6px` 位移和 `.64s` 过渡；落位仍为 `1 / 0`，不改变 DOM、IntersectionObserver、API、数据或业务交互。
- 5185 隔离实例真实确认 `scrollY=720` 预落位与 `scrollY=1200` settled、透明 AI TOKEN 顶栏、无横向溢出和清洁页面日志；新证据见 `ui-accessibility-evidence-v89.md`，viewport override、真实设备与辅助偏好继续独立门禁。

### v90 交付增量

- 周期切换器普通配色改为低 alpha 深蓝透景层与轻微 blur，active lime pill、period API、ARIA pressed、DOM 和数据请求契约不变；forced-colors 不进入玻璃规则。
- 5186 隔离实例真实确认首屏/滚动态、Today/Week/Month/All time 四档状态与 pill 几何、透明顶栏、无横向溢出和清洁页面日志；新证据见 `ui-accessibility-evidence-v90.md`，viewport override、真实设备与辅助偏好继续独立门禁。

### v91 交付增量

- sticky 避让候选扩展到 `.site-main .card-heading`；深度滚动时标题视觉退出 sticky 顶栏交叠带，返回后恢复，DOM、h1/h2 语义、API 和业务数据不变；`:focus-within` 保留标题内按钮操作。
- 5187 隔离实例真实确认 `scrollY=1118/898` 的避让与恢复、透明 AI TOKEN 顶栏、无横向溢出和清洁页面日志；新证据见 `ui-accessibility-evidence-v91.md`，viewport override、真实设备与辅助偏好继续独立门禁。

### v92 交付增量

- sticky 避让候选扩展到 `.site-main .auto-form-actions`，按钮与隐私说明作为完整操作行退出顶栏交叠带；`:focus-within` 保留提交按钮可见、可达、可操作，DOM、表单值、API 和业务数据不变。
- 5188 隔离实例真实确认 `scrollY=2200` 避让、Tab 聚焦恢复、离开交叠带恢复、透明 AI TOKEN 顶栏、无横向溢出和清洁页面日志；更上方表单字段避让、viewport override、真实设备与辅助偏好继续独立门禁。

### v93 交付增量

- 透明 AI TOKEN sticky 顶栏继续保持 `transparent / no blur`；新增内容表面交叠测量和局部裁剪，仅移除穿过 76px 顶栏带的卡片填充，背景场景恢复连续透景。
- `:focus-within` 清除卡片裁剪，自动采集备注输入仍可通过键盘聚焦并保持完整字段可见；不改变 DOM、Tab 顺序、API、认证、业务数据或 forced-colors/reduced-motion 复位边界。
- 5189 隔离实例真实确认 `scrollY=2200` 顶栏透明、卡片交叠裁切、备注字段焦点恢复、离开交叠带恢复、40 个可见焦点控件、1 个 `h1`/8 个 `h2`、无横向溢出和清洁页面日志；viewport override、真实设备与辅助偏好继续独立门禁。

### v94 交付增量

- 深滚时仅剩不超过 120px 的卡片孤立下沿现在通过 `is-sticky-fragment` 作为完整视觉单元退出，避免透明 AI TOKEN 行下出现第二条深色横带；完整卡片仍使用交叠区 mask 与短 feather。
- `:focus-within` 清除 fragment 隐藏并恢复卡片与动作行，真实 Tab 顺序保持可达；不改变 DOM、API、认证、表单值、数据流或系统配色复位。
- 5190 隔离实例真实确认 `scrollY=2200` 完整过渡后的卡片/提交行隐藏、备注字段焦点恢复、中间滚动主体可见、40 个可见焦点控件、1 个 `h1`/8 个 `h2`、无横向溢出和清洁页面日志；viewport override、真实设备与辅助偏好继续独立门禁。

### v95 交付增量

- 长页面 reveal 使用 scroll/resize 单帧同步器补齐漏掉的观察器回调；已越过阈值的区块统一清除延迟并进入 `is-visible`，不改变 DOM、API、认证、表单值、数据流或 reduced-motion/forced-colors 复位。
- 5191 隔离实例真实确认深度快跳约 `scrollY=2995` 与返回 `scrollY=1200` 后 8 个 reveal 区块均为 `opacity=1 / transform=none`，39 个交互控件具备名称、1 个 `h1`/8 个 `h2`、无横向溢出和清洁页面日志；真实设备与辅助偏好继续独立门禁。

### v97 交付增量

- 新增仅针对 `max-width:360px` 且 `max-height:760px` 的 hero 首屏契约，收紧窄短手机的 stage spacing 与轨道尺寸，使两个主操作完整进入视口；不改变 DOM、API、认证、表单值、数据流、桌面/平板布局或 reduced-motion/forced-colors 复位。
- 5193 隔离实例真实确认 `320×720` 主操作 `bottom=698`、六档响应式无横向溢出、41 个控件具备名称、1 个 `h1`/8 个 `h2` 和清洁页面日志；真实设备与辅助偏好继续独立门禁。

### v98 交付增量

- 指标栏 `.signal-cell` 使用低 alpha、无模糊局部透景层与轻分隔线，提升三组 token 指标在插画上的扫读稳定性；不改变透明 AI TOKEN 顶栏、DOM、API、认证、表单值、数据流、Tab 顺序或 forced-colors/reduced-motion 复位。
- 5194 隔离实例真实确认 `320/390/768/1024/1440` 五档无横向溢出、指标单元与顶栏均为 `backdrop-filter = none`、39/39 当前控件具备名称和清洁页面日志；真实设备与辅助偏好继续独立门禁。

### v99 交付增量

- AI TOKEN 品牌 header 在普通配色下维持 `transparent / no backdrop-filter / no box-shadow`；hero-topline 取消全宽底线，仅保留局部短信号线，避免最上方场景产生黑色覆盖错觉；不改变 DOM、API、认证、数据流或 forced-colors/reduced-motion 边界。
- 5195 隔离实例真实确认 `320/390/768/1024/1440` 五档无横向溢出、header/hero-topline 透明计算值、周期四档状态同步、36/36 当前控件具备名称和清洁页面日志；真实设备与辅助偏好继续独立门禁。

### v100 交付增量

- hero 元信息右侧 `#dashboard-status` 在普通配色下改为 `transparent / no backdrop-filter`，错误态继续保留专用边界和颜色；品牌副标题增加局部 keyline，不改变 DOM、API、认证、数据流或 forced-colors/reduced-motion 边界。
- 5200 隔离实例真实确认 `320/390/768/1024/1440` 五档无横向溢出、状态胶囊透明计算值、`scrollY=720` 滚动态、周期四档状态同步、40/40 当前控件具备名称和清洁页面日志；真实设备与辅助偏好继续独立门禁。

### v101 交付增量

- 登录认证卡片在普通配色下改为低 alpha 深蓝透景层与 `blur(14px) saturate(1.08)`，减少黑色表面覆盖并恢复背景场景参与；不改变 DOM、认证 API、字段、焦点顺序或 forced-colors/reduced-motion 边界。
- 5201 隔离实例真实确认 `320/390/768/1024/1440` 五档卡片几何、无横向溢出、聚焦边界、7/7 当前控件具备名称和清洁页面日志；真实设备与辅助偏好继续独立门禁。

### v102 交付增量

- Dashboard 周期切换器在普通配色下改为低 alpha 垂直渐变、无 blur 的局部透景层，保留 border、active lime、四档 period 语义、键盘状态和 forced-colors/reduced-motion 边界；不改变 DOM、API、认证、表单值、数据流或 Tab 顺序。
- 5202 隔离实例真实确认 `320/390/768/1024/1440` 五档无横向溢出、Today/Week/Month/All time 四档真实点击与 `aria-pressed` 同步、36/36 或 40/40 当前控件命名、1 个 h1/8 个 h2、桌面与深滚截图和清洁页面日志；证据见 `ui-accessibility-evidence-v102.md`，真实设备与辅助偏好继续独立门禁。

### v103 交付增量

- 登录与注册认证卡片在普通配色下改为 `.54/.66` alpha 深蓝透景层与 `blur(11px) saturate(1.06)`，保留字段局部阅读底、焦点边界、顶部高光和 forced-colors/reduced-motion 边界；不改变 DOM、认证 API、字段、Tab 顺序或数据流。
- 5203 隔离实例真实确认 `/login` 与 `/register` 在 `320/390/768/1024/1440` 五档无横向溢出、卡片 `blur(11px)`、7/7 当前控件命名、1 个 h1/1 个 h2、登录字段焦点和桌面/移动截图；证据见 `ui-accessibility-evidence-v103.md`，真实设备与辅助偏好继续独立门禁。

### v104 交付增量

- Dashboard 图表、自动采集、说明、记录和手动明细卡片在普通配色下改为 `.70/.82` alpha、`blur(12px) saturate(1.06)` 深蓝透景层；保留字段局部阅读底、空态图表语义、焦点边界、forced-colors/reduced-motion 边界，不改变 DOM、API、认证、表单值、数据流或 Tab 顺序。
- 5204 隔离实例真实确认 `320/390/768/1024/1440` 五档无横向溢出、卡片 computed style、Today/Week/Month/All time 四档真实点击、Base URL 字段焦点、36/37 或 40/41 当前控件命名、1 个 h1/8 个 h2、桌面/移动深滚截图和清洁页面日志；证据见 `ui-accessibility-evidence-v104.md`，真实设备与辅助偏好继续独立门禁。

### v105 交付增量

- 自动采集标题旁的 `connection-badge` 在普通配色下改为低 alpha、无 blur 的局部透景层，保留未连接、ready、error 三类状态边界和颜色，不改变 DOM、API、认证、表单值、数据流、Tab 顺序或 forced-colors/reduced-motion 边界。
- 5205 隔离实例真实确认 `320/390/768/1024/1440` 五档无横向溢出、状态 `backdrop-filter: none`、Dashboard 卡片 `blur(12px)`、Base URL 字段焦点、36/37 或 40/41 当前控件命名、1 个 h1/8 个 h2、深滚截图和清洁页面日志；证据见 `ui-accessibility-evidence-v105.md`，真实设备与辅助偏好继续独立门禁。

### v106 交付增量

- `.hero-topline` 的四边边框在普通配色下统一透明，局部 signal trace 仍提供轻量结构提示；不改变 DOM、API、认证、表单值、数据流、Tab 顺序或 forced-colors/reduced-motion 边界。
- 真实隔离 5206 实例确认五档响应式无横向溢出、metadata `background: transparent` / `backdrop-filter: none` / 四边透明、周期 `aria-pressed` 同步、深滚 header/card 层级和清洁页面日志；证据见 `docs/ui-accessibility-evidence-v106.md`。

### v107 交付增量

- `.hero-lede` 增加局部文字 keyline，保护透明插画上的中文说明阅读边缘；不改变 DOM、API、认证、表单值、数据流、Tab 顺序或 forced-colors/reduced-motion 边界。
- 真实隔离 5208 实例确认 `320×720/390×844/768/1024/1440` 五档无横向溢出、说明文字阴影、metadata 四边透明、周期 `aria-pressed` 同步、深滚 header/card 层级和清洁页面日志；证据见 `docs/ui-accessibility-evidence-v107.md`。

### v108 交付增量

- 登录/注册共享认证卡片的辅助说明与底部切换链接使用更高语义文字色、局部文字 keyline 和细 lime 下划线；不改变 DOM、API、认证流程、Tab 顺序或 forced-colors/reduced-motion 边界。
- 真实隔离 5209 实例确认登录/注册 `320/390/768/1440` 四档无横向溢出、卡片 `blur(11px)` 透明材质、链接样式、用户名 `:focus-within` 状态和清洁页面日志；证据见 `docs/ui-accessibility-evidence-v108.md`。

### v109 交付增量

- Dashboard 周期切换器加入既有 sticky occlusion 候选，透明 AI TOKEN 顶栏发生实际覆盖时收束并停止点击，`focus-within` 恢复可见和可操作；不新增监听器，不改变周期 API、DOM 语义或 Tab 顺序。
- 真实隔离 5210 实例确认 `320/390/768/1024/1440` 五档无横向溢出、交叠/非交叠边界、焦点恢复、1 个 h1/8 个 h2 和清洁页面日志；证据见 `docs/ui-accessibility-evidence-v109.md`。

### v110 交付增量

- 顶部 AI TOKEN 品牌栏统一使用低 alpha 透光渐变，普通配色下无 blur、无深色外投影，forced-colors 分支仍交给系统颜色；证据见 `docs/ui-accessibility-evidence-v110.md`。
- 真实隔离 5211 实例确认登录页、Dashboard 首屏/`scrollY=1200` 深滚、`390×844` 与 `320×720` 的 computed style、无横向溢出和清洁页面日志。
### v111 交付增量

- 认证卡片既有顶部 signal line 增加低频边界呼吸；仅普通配色/允许动效启用，聚焦、forced-colors、reduced-motion 不改变可访问边界。
- 真实隔离 5212 实例确认桌面与 `390×844` 的 computed style、焦点边界、卡片几何、可访问性结构、无横向溢出和清洁页面日志；证据见 `docs/ui-accessibility-evidence-v111.md`。
### v112 交付增量

- Dashboard 指标栏复用 sticky occlusion 候选机制；三个 `.signal-cell` 穿过透明 AI TOKEN 顶栏时淡出并停止指针命中，离开交叠区后恢复，不改变数据、API、DOM 语义或移动端列布局。
- 真实隔离 5213 实例确认 `1440×900` 首屏与稳定 `scrollY=980` 深滚的顶栏 `76px` 边界、指标交叠隐藏、无横向溢出和完成态页面；证据见 `docs/ui-accessibility-evidence-v112.md`。
### v113 交付增量

- 普通配色下 AI TOKEN 顶栏使用真正透明窗口，移除渐变、内侧阴影和模糊层；保留文字 keyline、滚动进度线、forced-colors 和 reduced-motion 边界，不改变模板、API 或认证流程。
- 真实隔离 5214 实例确认登录页、Dashboard 首屏/深滚及 `320/390/768/1024/1440` 五档无横向溢出、标题层级、控件命名和清洁页面日志；证据见 `docs/ui-accessibility-evidence-v113.md`。
### v114 交付增量

- 共享导航在 body 高度发生异步变化时自动重算滚动比例；品牌进度线使用固定宽度 `scaleX`，不改变 DOM、API、认证流程或用户滚动语义。
- 真实隔离 5215 实例确认异常补录展开后的文档高度/进度同步、`320/390/768/1024/1440` 五档无横向溢出、稳定深滚比例和清洁页面日志；证据见 `docs/ui-accessibility-evidence-v114.md`。
### v115 交付增量

- 透明 AI TOKEN 顶栏拆出 `header-chrome.css`：以无填充 composited hairline、品牌标记 hover/focus 和文字 keyline 提供层级，不恢复黑色面板、blur 或 shadow。
- 真实隔离 5216 实例确认 `390×844` 无横向溢出、`1440×900` 登录/Dashboard 首屏、`scrollY=2360` 深滚透明顶栏和进度线；证据见 `docs/ui-accessibility-evidence-v115.md`。
### v116 交付增量

- Dashboard 空态趋势图与模型占比改为 observatory signal bay；空态 status、自动采集入口、Chart.js canvas 状态和周期控件语义保持。
- 真实隔离 5217 实例确认 `320/390/768/1024/1440` 五档无横向溢出、`scrollY=820` 深滚空态、`role=status`、Week `aria-pressed` 和清洁页面日志；证据见 `docs/ui-accessibility-evidence-v116.md`。

### v117 交付增量

- 最上方 AI TOKEN 品牌栏新增最终透明契约：普通配色下 header 与子级 `background: transparent`、`background-image: none`、无 blur、无 shadow；保留文字 keyline、hairline、滚动 signal line，不改变认证、导航、DOM 或 API。
- 真实隔离 5218 实例确认 `320/390/768/1024/1440` 五档无横向溢出、登录首屏/滚动态/390px 截图、透明 computed style 和清洁应用日志；证据见 `docs/ui-accessibility-evidence-v117.md`。

### v118 交付增量

- 登录/注册认证字段新增装饰性 `IDENTITY / 01`、`ACCESS / 02` 元信息、focus-within 局部 signal line 和低 alpha 深蓝阅读面；元信息使用 `aria-hidden`，不改变输入可访问名称、认证接口或数据流。
- 真实隔离 5219 实例确认 `320/390/768/1024/1440` 五档无横向溢出、登录/注册继承、真实字段焦点、`1 个 h1 / 1 个 h2` 和清洁应用日志；证据见 `docs/ui-accessibility-evidence-v118.md`。

### v119 交付增量

- `03 / AUTOMATIC COLLECTION` 旁的 `HOW IT WORKS` 说明在 `901px+` 变为 `position: sticky; top: 96px` 的 protocol rail，使检测、代理、usage 归档三步协议在长表单中段持续可见；901px 以下恢复普通文档流。
- 真实隔离 5220 实例确认 `320/390/768/1024/1440` 五档无横向溢出、1440px sticky 深滚、Base URL 焦点恢复、390px 长表单和清洁应用日志；证据见 `docs/ui-accessibility-evidence-v119.md`。

### v120 交付增量

- 基础 `style.css` 补回共享场景背景的兼容边界，确保 `.site-header` 的透明材质确实透出背景图，而不是落到黑色 body 画布；完整样式链继续保留既有 scene motion 覆盖与无障碍降级。
- 真实 `5000` 页面与隔离 `5221` 确认 `320/390/768/1024/1440` 五档无横向溢出、背景图加载、透明 computed style、1440px 滚动态和清洁应用日志；证据见 `docs/ui-accessibility-evidence-v120.md`。

### v121 交付增量

- `620px` 以下认证页面减少介绍区到登录/注册卡片的垂直空隙，移动用户更快抵达主要输入任务；不改变标题层级、字段命名、认证请求或桌面构图。
- 真实 `5000` 确认登录/注册在 `320/390/768/1024/1440` 五档无横向溢出、390px 卡片提前、键盘焦点和清洁应用日志；证据见 `docs/ui-accessibility-evidence-v121.md`。
### v122 交付增量

- Dashboard 在 `620px` 以下将 orbit 收敛为视觉锚点，压缩 hero stage 的无效高度，让移动端首次扫描连续经过总量轨道、标题、主要入口和周期控制；桌面构图、模板语义、API 和业务数据不变。
- 真实 `5000` 确认 `320×720/320×844/390×844/768×900/1024×900/1440×900` 六档无横向溢出，390px 首屏入口与周期控制提前，Week 的 `aria-pressed` 与状态文案正常，深滚空态和应用日志正常；证据见 `docs/ui-accessibility-evidence-v122.md`。

### v123 交付增量

- 基础 `style.css` 补齐旧个人启动器的 sticky occlusion 视觉契约，使深滚分析标题在透明 AI TOKEN 阅读带内淡出，卡片 surface 使用既有自定义变量 mask，焦点内容恢复可见；顶栏仍保持透明、无 blur、无 shadow。
- 真实 `5000` 确认深滚标题/卡片层级、六档响应式、Week/键盘焦点、`1 个 h1 / 8 个 h2` 和清洁应用日志；证据见 `docs/ui-accessibility-evidence-v123.md`。

### v124 交付增量

- 导航模块为初始分享 hash 增加异步高度稳定定位，确保 `#connect/#activity/#history` 直接打开时章节标题位于透明 AI TOKEN 顶栏下方；用户交互立即取消自动校正，普通平滑导航不变。
- 全新浏览器上下文确认三个章节最终落点、无 hash 首屏、Connect 点击导航、滚动策略释放、横向溢出和清洁应用日志；证据见 `docs/ui-accessibility-evidence-v124.md`。

### v125 交付增量

- 最上方 AI TOKEN 品牌行改为低 alpha 深蓝玻璃层：保留背景透出、轻量 `blur(11px)`、边界高光和移动端可读性增强；`ui-polish.css` 为旧启动器提供同契约回退，不再由最后一层 CSS 强制清空材质，也不引入实色黑色面板。
- 同步修正 `header-chrome.css`、`ui-polish.css`、基础样式及证据注释；5000/5011 样式资源返回正常，Python 编译、Git 空白和文本文件行数门禁通过，保护服务未重启；证据见 `docs/ui-accessibility-evidence-v125.md`。

### v126 交付增量

- 旧个人入口的 `AI TOKEN / OBSERVATORY` 元信息行移除全宽底线，改为左侧 `220px` 局部 signal trace；背景插画连续透出，forced-colors 下恢复系统边界。
- 真实 `5000` 桌面与 `390×844` 页面确认透明元信息层、局部 trace、原有首屏布局、无横向溢出和清洁运行状态；详见 `docs/ui-accessibility-evidence-v126.md`。
- 因通用 `ui-polish.css` 达到 1019 行，本轮将 Admin 专属视觉规则拆至 `admin-polish.css`，由 `admin.html` 独立加载；审计恢复 `14 pass / 1 pending / 0 fail`，`ui-polish.css=867`、`admin-polish.css=90`。

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

## v127 顶部品牌行连续透景

- UI-1：修复根目录旧个人启动器中顶部 `AI TOKEN / OBSERVATORY` 因高 alpha wash 看起来像黑色横条的问题，恢复导航 chrome 与背景插画的连续关系。
- UI-2：`ui-polish.css` 兼容层将 header wash 收敛为 `.07/.16/.035`；`brand-transparency.css` 同步完整链和窄屏渐变，保留 `blur(11px)`、文字 keyline、signal line、forced-colors 与 reduced-motion 边界。
- UI-3：真实 `5000` 桌面 `1683×892` 与移动 `390×844` 已确认 computed style、背景连续、品牌可读、无正向横向溢出和清洁日志；证据见 `ui-accessibility-evidence-v127.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Provider 或 Key 生命周期改动；保护端口未重启。
- 架构师：变化限定在 `01-shell / sticky brand transparency compatibility boundary`，无新依赖、无跨层耦合；`ui-polish.css=867`、`brand-transparency.css=94`，回滚边界为两处 header 背景声明。

## v128 首屏 CTA 滚动态收束

- UI-1：修复旧入口滚动后绿色主 CTA 片段留在透明 AI TOKEN 品牌行下方、抢占阅读焦点的问题。
- UI-2：`ui-polish.css` 复用既有 `site-header.is-scrolled` 状态；非焦点 `.hero-actions` 和 `.scroll-cue` 以受控过渡退出，`:focus-within` 保留键盘路径，周期切换器不受影响。
- UI-3：真实 `5000` `390×844` 与 `1683×892` 首屏/深滚状态已确认 computed style、周期控件可用、无正向横向溢出和清洁日志；证据见 `ui-accessibility-evidence-v128.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Provider 或 Key 生命周期改动。
- 架构师：变化限定在 `01-shell / legacy launcher sticky reading boundary`，无新依赖、无跨层耦合；回滚边界为 `ui-polish.css` v128 兼容块。

## v129 观测区深蓝信号窗口

- UI-1：修复周期切换器、指标轨与图表空态之间的材质断层，避免观测区被读成两个重黑卡片。
- UI-2：周期切换器与图表面使用低 alpha 深蓝渐变、轻量 `blur(8px)`、网格、局部 signal line 和文字 keyline；旧入口兼容规则拆至 `legacy-observatory.css`，完整链由 `observatory-signal.css` 持有，不改数据/Chart.js/API 契约。
- UI-3：真实 `5000` `390×844`/`1683×892` 首屏、深滚、空态、Week 键盘切换和清洁日志已确认；证据见 `ui-accessibility-evidence-v129.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Provider 或 Key 生命周期改动。
- 架构师：变化限定在 `03-observatory / legacy presentation boundary`，`ui-polish.css=872`、`legacy-observatory.css=131`、`observatory-signal.css=196`，均低于 1000 行。

## v130 AI TOKEN 顶部清透轻玻璃

- UI-1：修复旧体验入口顶部品牌行像黑色覆盖带的问题；背景插画、品牌锁定组和导航重新形成连续透景。
- UI-2：共享 `ui-polish.css` 与 `brand-transparency.css` 统一为 `.015` 背景色、`5% → 0%` 渐变、`blur(4px) saturate(1.03)` 和弱边界阴影；窄屏增强同步降低，系统高对比度仍由原规则接管。
- UI-3：真实 `5000` 桌面/移动/滚动态与 `5011` 完整链已确认计算样式、无新增正向横向溢出、观测区构图未回退和清洁应用日志；证据见 `docs/ui-accessibility-evidence-v130.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Provider 或 Key 生命周期改动；保护端口未重启。
- 架构师：变化限定在 `01-shell / brand transparency boundary` 的两个高内聚 CSS 文件，无新依赖、DOM、脚本或跨层耦合；所有文本源码继续低于 1000 行。

## v131 长页面深蓝观测表面

- UI-1：修复图表观测窗与自动采集/活动/历史大卡片之间的材质断层，长页面保持同一套深蓝观测语言，背景设备继续透出。
- UI-2：旧入口新增 `legacy-observatory.css` 长页面 surface block；普通卡片 `.72/.84`、活动采集 `.76/.88`，移动端 `.78/.90` 与 `.82/.92`，统一 `blur(10px) saturate(1.04)`；不改字段、数据或布局。
- UI-3：真实 `5000` 桌面/390px 长滚、`5011` 完整链、Week 点击、手动补录展开和清洁日志已确认；证据见 `docs/ui-accessibility-evidence-v131.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Provider 或 Key 生命周期改动；保护端口未重启。
- 架构师：变化限定在 `03-observatory / legacy presentation compatibility boundary`，单文件保持低于 1000 行，forced-colors/reduced-motion 与焦点恢复边界保留。

## v132 末端空态与终止信号

- UI-1：修复旧入口活动历史空态在手机上横向滚动、文案被截断的问题；最近记录与页脚形成明确的 Dashboard 终止层级。
- UI-2：仅 `.activity-history-card.is-empty` 取消 populated table 的 `540px` min-width 并隐藏表头；新增 footer terminal band、局部 signal trace、低 alpha blur；forced-colors 恢复系统色。
- UI-3：真实 `5000` 的 320/390/768px 与桌面末端、`5011` 完整链、空态文案、卡片溢出和清洁日志已确认；证据见 `docs/ui-accessibility-evidence-v132.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Provider 或 Key 生命周期改动；保护端口未重启。
- 架构师：变化限定在 `03-observatory / legacy presentation compatibility boundary`，不改变 populated table、DOM、脚本或数据契约；文件仍低于 1000 行。

## v133 AI TOKEN 品牌行完全透景

- UI-1：修复最上方 `AI TOKEN` 行在旧入口和完整入口中像深色覆盖带的问题；品牌栏现在完全透过背景场景，文字 keyline、进度线和焦点状态保持可读。
- UI-2：`ui-polish.css` 与 `brand-transparency.css` 统一为普通配色 `transparent / none / no blur / no shadow`；其它 header 模块只维护边界和职责说明，不新增 DOM、脚本或依赖。
- UI-3：真实 5000 桌面首屏/深滚、5011 完整链 computed style、截图、DOM 快照、无正向横向溢出和清洁日志已确认；证据见 `ui-accessibility-evidence-v133.md`。
- 后端：无后端、认证、数据库、CSRF、Provider 或数据契约变化；保护 PID 保持不变。
- 架构师：变化限定在 `01-shell / brand transparency boundary`，`ui-polish.css=919`、`brand-transparency.css=100`，所有修改文件均低于 1000 行；正式 Android、EXE 签名、HTTPS/ACL、真实 Provider、备份恢复和真实设备门禁继续 pending。

## v134 Hero 元信息局部信号胶囊

- UI-1：修复 Dashboard 首屏更新状态在插画上不易扫描的问题；状态胶囊增加局部阅读层，产品标签增加 beacon，整条 hero 元信息带和 AI TOKEN 顶栏不恢复为黑色横条。
- UI-2：`hero-signal.css` 作为独立 02-observatory 表现模块接入旧入口兼容链；复用既有节点、CSS 状态和系统动效边界，不新增业务逻辑。
- UI-3：真实桌面、390px、320px、5011 深滚、无正向横向溢出、DOM/ARIA 和清洁日志已确认；证据见 `ui-accessibility-evidence-v134.md`。
- 后端：无后端、认证、数据库、CSRF、Provider 或数据契约变化；保护 PID 保持不变。
- 架构师：新增 `hero-signal.css=163`、`ui-polish.css=920`，均低于 1000 行；真实设备、Android、EXE 签名、HTTPS/ACL、Provider、备份恢复门禁继续 pending。

## v135 移动端空态图表尺寸契约

- UI-1：修复 320/390px 空态图表卡片内部隐藏横向通道；off-canvas sheen 已移除，空态 Chart.js canvas 与当前 wrapper 宽度一致，空态文案与信号网格保持可读。
- UI-2：仅 `empty` / `unavailable` 状态使用 `overflow: hidden`、canvas 收缩和 forced-colors sheen 关闭；populated chart、周期控制、DOM、脚本和数据契约保持不变。
- UI-3：真实 `5000` `320×720`、`390×844`、`1683×892` 与 `5011` `390×844` 的 computed style、截图、无正向横向溢出和应用侧日志已确认；证据见 `ui-accessibility-evidence-v135.md`。
- 后端：无后端、认证、数据库、Provider、API 或数据契约变化；保护 PID 保持不变。
- 架构师：变化限定在 `03-observatory / legacy presentation compatibility boundary`，修改文件低于 1000 行，正常配色/forced-colors/reduced-motion 边界完整。

## v136 长页面 reveal 可读性契约

- UI-1：修复 Dashboard 章节在 reveal 未完成时过暗、像黑色覆盖层的问题；预显隐改为 `.78` 与 `6px`，仍保留轻量深度，不牺牲背景透景和阅读层级。
- UI-2：仅调整共享 `data-reveal` CSS contract 并删除重复覆盖，`prefers-reduced-motion` 仍直接显示完整内容；业务 DOM、脚本、图表、表单和数据契约保持不变。
- UI-3：真实 `5000` `1683×892`/`390×844`/`320×720`/`768×900`、`scrollY=1200` 与 `5011` `1440×900` 的 computed style、截图、无正向横向溢出和页面侧日志已确认；证据见 `ui-accessibility-evidence-v136.md`。
- 后端：无后端、认证、数据库、Provider、API 或数据契约变化；保护 PID 保持不变。
- 架构师：变化限定在 `01-shell / shared reveal presentation boundary`，无新依赖、重复模块或跨层耦合，所有修改文件低于 1000 行。

## v137 Activity 导航兼容补链

- UI-1：Activity 工作信号区现在在旧缓存入口中也有明确顶部导航入口；四项导航在桌面/平板可见，移动端按既有规则隐藏。
- UI-2：兼容 guard 只在页面存在 `#activity` 且导航缺失时插入链接；重复检测、路径、hash、active state、ARIA 与现有滚动契约保持一致。
- UI-3：真实 `5000` `1440×900`/`1024×900`/`390×844`、Activity 深链接与 `5011` `1024×900` 的导航计数、定位、透明顶栏、无正向横向溢出和页面侧日志已确认；证据见 `ui-accessibility-evidence-v137.md`。
- 后端：无后端、认证、数据库、Provider、API 或数据契约变化；保护 PID 保持不变。
- 架构师：变化限定在 `01-shell / navigation compatibility boundary`，无新依赖，`navigation.js` 仍低于 1000 行。

## v138 手机首屏周期切换器收口

- UI-1：390×844 首屏周期切换器从视口外沿收回到 `bottom=842.61`，Today 活动项完整可读；背景、品牌栏和 hero 动画不被重新着色。
- UI-2：新媒体查询仅作用于正常高度手机；320×720、768×900、1440×900 保持既有布局，周期交互与 ARIA 契约不变。
- UI-3：真实 5000/5011 入口的 390×844、5000 的短屏/平板/桌面、`aria-pressed`、文档无横向溢出、共享样式来源和页面侧日志已确认；证据见 `ui-accessibility-evidence-v138.md`。
- 后端：无后端、认证、数据库、Provider、API 或数据契约变化；保护 PID 保持不变。
- 架构师：变化限定在两个启动链共同加载的共享 presentation boundary，未加载的旧候选已撤回，所有修改文件低于 1000 行。

## v139 窄屏场景焦点收口

- UI-1：移动场景焦点从 68%/旧入口 40% 收敛为共享 30%，消除背景高亮穿过标题和 CTA 的视觉竞争；源图右侧人物、笔记本与服务器仍保留为叙事锚点。
- UI-2：完整链的后置 68% 规则由共享 `body .story-backdrop` 合法覆盖；无 `!important`，桌面/平板不命中，reduced-motion/forced-colors 边界继续存在。
- UI-3：真实 5000/5011 的 390×844、5000 的 320×720/768×900/1440×900、透明顶栏、按钮 `aria-pressed`、文档宽度和页面侧日志已确认；证据见 `ui-accessibility-evidence-v139.md`。
- 后端：无后端、认证、数据库、Provider、API 或数据契约变化；保护 PID 保持不变。
- 架构师：变化限定在共享场景 presentation boundary，兼容旧入口真实加载链，所有修改文件低于 1000 行。

## v140 旧入口场景动效兼容补链

- UI-1：5000 旧链从 `animation-name: none` 补为低强度 `shared-scene-glow/shared-scene-scan`；5011 保持正式 `backdrop-glow/backdrop-scan`，旧链视觉生命感与完整链对齐。
- UI-2：新增动效仅作用于 `aria-hidden` 背景伪元素，使用 18s/21s 慢节奏和低透明度，`prefers-reduced-motion` 停止，`forced-colors` 隐藏；不影响阅读层、焦点或文档几何。
- UI-3：真实两个入口的 390/320/768/1440 断点、动效计算值、无障碍树、透明顶栏、周期 ARIA、无横向溢出和页面日志已确认；证据见 `ui-accessibility-evidence-v140.md`。
- 后端：无后端、认证、数据库、Provider、API 或数据契约变化；保护 PID 保持不变。
- 架构师：变化限定在共享 scene presentation boundary，无新依赖、重复逻辑或跨层耦合，所有修改文件低于 1000 行。
