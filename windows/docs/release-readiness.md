# 发布就绪审计（持续更新）

**作者：** AI Token Tracker Engineering Team
**维护者：** Project Owner
**状态：** In Progress；未达到最终交付条件
**最后核对：** 2026-08-12

## 目的

本文件是架构师交付交接物，不是“源码能启动”证明。只有表格中的所有必要项都达到
`pass`，并且真实运行证据与部署负责人记录齐全，才允许把长期目标标记为完成。

逐条需求、证据和剩余门禁的唯一核对表见 [`final-acceptance-matrix.md`](final-acceptance-matrix.md)。

## 当前门禁矩阵

| 范围 | 当前状态 | 证据/下一步 |
| --- | --- | --- |
| Windows 模块化单体、CLI、SQLite、Web、RBAC | pass（源码/本机运行） | `compileall`、CLI help、health、根 `run.py` shim、CLI/根入口 Waitress 本机 WSGI、个人入口与 CLI loopback fail-closed、API/安全边界和架构文档已核对（ADR-078/094） |
| 自动 Provider usage / Gateway | 条件通过 | Gateway、SSE usage、Usage Ingest、DPAPI 重试队列、运行时队列故障 fail-closed、动态中心端口只读发现、响应读取/幂等键/非回环 HTTPS 门禁已完成（ADR-058/059/068/075）；仍需用户本人合法 Provider Key 做一次非流式/流式联调 |
| Web UI-1/UI-2 | pass（源码/本机视觉） | v14 场景、v29 左上遮罩/导航玻璃层、v31 登录首帧与左上环境光运行观察、v32 Admin 共享 pointer/backdrop/reveal/surface motion 与动态详情可读性、v33 顶部半透明玻璃导航与滚动章节状态、v34 自动采集说明卡片表面收口、v35 待显区块可读性和锚点动画收束、v36 表格零信号空态信标与扫描线、v37 周期切换 active pill 与 pressed 状态同步、v38 select 暗色玻璃表面与箭头、v39 顶部 AI TOKEN 品牌栏透景遮罩收敛与滚动保持、v40 异常补录 disclosure 状态层次与展开过渡、v41 章节锚点安全落点与 sticky header 协同、v42 数据面板阅读遮罩与空/非空状态、桌面/320px 复核、ADR-074 可见度基线、ADR-095 场景资产决策、显式主图层、动效和首帧可读性已在临时源码实例复核；v13/v12/v11/v10/v9/v8 保留回滚 |
| Web UI-3 | in progress | 登录页四档观察（v27/v29/v31）、Dashboard v30 空态行动入口/非空图表回归、Admin v31 数据轨道/详情焦点回流、v32 动态详情打开后的可读性、焦点回流和清洁控制台、v33 导航 active/aria-current、滚动进度线、顶部半透明玻璃层和 320px 无溢出、v34 自动采集双栏收口、320px 单列和清洁控制台、v35 待显区块可读性、锚点进入视口后的动画收束和 320px 无溢出、v36 Dashboard/Activity/Admin 表格零信号空态、微动效和 320px 无溢出、v37 四周期 active pill、aria-pressed、锚点/几何同步和 320px 无溢出、v38 暗色 select、焦点样式、实际选项切换和 320px 无溢出、v39 顶部品牌栏透明层、滚动保持和 320px 无溢出、v40 异常补录 disclosure 的 Space/Enter 交互、焦点和 320px 无溢出、v41 Analysis/Connect/Activity 安全落点、History 底部边界和移动端无溢出、v42 图表/活动/历史数据面空/非空状态与对比度遮罩、320px 无溢出、认证页 v23 唯一语义 h1/aria-hidden glitch 层、隔离合法会话下 Dashboard/Admin 空态、成员脱敏日志详情、动态错误 alert/status 播报、历史非空、服务端 CSV 200、Provider 502 错误态、个人 Activity 写入切片和 v14 默认浏览器首屏均已有源码/隔离证据；真实设备指标、浏览器下载落盘、真实 Provider 成功、reduced-motion/高对比度环境仍待证据 |
| EXE | conditional | PyInstaller 6.22.0 已锁定并按 v14 资源重建 onedir EXE/ZIP；v14 包 verifier、`/login`、`/api/v1/ready`、v14 资源 200、隔离 `%LOCALAPPDATA%` 建库和 EXE 目录无 data 已通过；v13→v14→v13 真实 API 升级/回滚与记录保留已通过（ADR-091/092/096）；签名入口/验证器已 fail-closed 接入（ADR-093），实际 Authenticode 和正式分发仍待验收 |
| Android APK | pending | 项目内 JDK 17、Gradle 9.5.0、command-line tools 和官方 Wrapper 已准备并由 doctor 识别；`accept-sdk-license.bat` 已提供用户授权入口；Android v14 背景与 SignalOrbit 已接入系统减少动画边界（ADR-079/095），API 37/Build Tools 因 SDK license 尚未交互确认仍 pending，APK/设备联调未验收（ADR-081/098） |
| HTTPS 中心部署 | pending | 应用生产绑定已强制 loopback（ADR-061）；Caddy v2.11.4 已在项目缓存中完成 SHA-512、版本和示例配置 validate，配置预检与显式 edge 启动器已接入（ADR-083）；2026-08-10 的 LAN handoff 与示例 Production edge preflight 均通过，但正式域名、ACL、证书、日志轮转和外部 health/ready 检查仍需在部署主机完成（ADR-097） |
| 备份恢复/回滚 | 条件通过 | 已填充隔离 EXE 数据库完成备份、`verify-backup`、`backup-inventory --verify` 和 staging 恢复二次校验，当前 schema（含 Usage Ingest Token）与防覆盖命名仍受门禁保护（ADR-060/077/088）；真实数据恢复、负责人确认的保留周期/离线副本和回滚记录仍待部署演练 |
| 工程质量门禁 | pass（当前 checkout） | `token_tracker audit --json`：代码/文本 1000 行门禁、作者头、Python docstring、Android KDoc、Web UI 静态契约、production/Caddy/LAN 部署契约、层间依赖边界、契约引用、跨端资产和 Chart.js 供应链通过；`windows/ci/quality-gate.ps1` 与 GitHub Actions 已接入（ADR-085）；`release-doctor` 提供统一只读编排入口 |

## v43 视觉增量

顶部 `AI TOKEN` 品牌栏已经移除深色底色，保留轻量透景 blur、底线和文本阴影；Token 历史账本补充桌面扫描层级与
320px 双列卡片布局。隔离浏览器证据、空态回归与未关闭真实设备门禁见 [`ui-accessibility-evidence.md`](ui-accessibility-evidence.md)
的 v43 章节。

## v44 视觉增量

sticky header 在首屏保持透明，滚动后切换为低透明度阅读玻璃层；导航滚动状态由共享 `navigation.js` 调度，认证页与受保护页面
保持一致。桌面/320px 隔离浏览器验证和未关闭真实设备门禁见 `ui-accessibility-evidence.md` 的 v44 章节。

## v45 视觉增量

滚动态 AI TOKEN 品牌栏改为透明羽化渐变，并移除滚动态背景 blur，避免 hero 大字在 sticky header 中形成白色雾带；首屏透明态、
导航语义、滚动状态调度和移动端无溢出保持不变。桌面/320px 隔离浏览器验证和未关闭真实设备门禁见
`ui-accessibility-evidence.md` 的 v45 章节。

## v46 视觉增量

首屏“开始自动采集”与补录深链接复用统一 `scroll-margin-top` 安全落点，避免目标卡片标题进入 sticky header 下方；桌面/320px
平滑滚动、无溢出和本地控制台证据见 `ui-accessibility-evidence.md` 的 v46 章节。

## v47 透明品牌栏基础态

顶部 AI TOKEN 品牌栏基础态移除背景 blur 与外部阴影，使背景图能够连续透景；细边线、内侧高光和文字阴影继续提供最低限度的结构与可读性。桌面、320px、滚动态、无溢出和本地控制台证据见 `ui-accessibility-evidence.md` 的 v47 章节。

## v48 登录表面半透明

登录/注册卡片降低背景梯度 alpha 与 blur 强度，使角色场景真正透过表面，同时保留文字可读性和高对比度/forced-colors 兜底。四档响应式、注册页复用、无溢出和本地控制台证据见 `ui-accessibility-evidence.md` 的 v48 章节。

## v49 交互态指针收敛

交互目标上的鼠标光晕与指针环降低尺寸、亮度和放大比例，保留 pointer follower、surface spotlight 与按钮磁吸，同时让原生焦点边界成为主要反馈。桌面交互态、离开态回收和本地控制台证据见 `ui-accessibility-evidence.md` 的 v49 章节。

## v50 滚动态品牌栏

AI TOKEN 品牌栏首屏继续完全透明；用户滚动进入分析区后才启用半透明渐隐层与轻微背景模糊，避免 sticky header 叠住统计文案，同时保留背景图透景和页面层次。桌面截图、computed style、skip-link 焦点、无横向溢出和控制台证据见 `ui-accessibility-evidence.md` 的 v50 章节；移动设备与辅助偏好矩阵仍是未关闭的正式验收项。

## v51 深链接 reveal 收束

Analysis/Activity/History 深链接目标不再叠加文档顺序 reveal 的 `350ms` 等待；目标 section 的透明度/位移在进入落点时以 `.56s` 完成，普通滚动 reveal 保持原节奏。真实桌面浏览器证据、无溢出、焦点与控制台结果见 `ui-accessibility-evidence.md` 的 v51 章节；移动设备、reduced-motion 和高对比度矩阵仍是未关闭的正式验收项。

## v52 移动端 CTA 首屏可见性

320px 下 hero 的两个关键入口改为等宽两列，避免“查看分析”因 flex 换行落到首屏底部之外；768px 与桌面维持原有横向节奏。真实 320/768/默认桌面截图、几何、焦点、无溢出和控制台证据见 `ui-accessibility-evidence.md` 的 v52 章节；1024/1440、真实设备和辅助偏好矩阵仍是未关闭的正式验收项。

## v53 顶部品牌栏透明层

AI TOKEN 顶部品牌栏的首屏与滚动态均移除黑色渐隐层和 blur，恢复背景场景透视，仅保留细边线与滚动进度线。真实 computed style、滚动截图和控制台证据见 `ui-accessibility-evidence.md` 的 v53 章节；完整响应式、真实设备、辅助偏好和正式部署验收仍未关闭。

## v54 平板首屏 CTA 可见性

621–900px 平板 hero 通过独立响应式层压缩轨道与垂直留白，使主 CTA 在 768×900 和 900×900 首屏完整可见；320、1024、1440 保持既有布局。真实五档几何、截图、焦点和控制台证据见 `ui-accessibility-evidence.md` 的 v54 章节；真实设备、辅助偏好和正式部署验收仍未关闭。

## v55 顶栏透明层与认证首屏

AI TOKEN 品牌栏恢复为低 alpha 透景玻璃层，首屏与滚动态分别使用轻量渐变和受控 blur；390px 手机登录/注册表单提前到介绍区之前，提交按钮进入首屏，桌面分栏保持不变。真实四档认证几何、滚动 computed style、截图和控制台证据见 `ui-accessibility-evidence.md` 的 v55 章节；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v56 移动 Dashboard 首屏节奏

320/390px Dashboard 缩小轨道并压缩文案留白，使主要操作与首屏底边保持约 48px 呼吸空间；768px 以上保持既有布局。真实五档几何、截图和控制台证据见 `ui-accessibility-evidence.md` 的 v56 章节；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v57 Dashboard 信号层可读性

状态文案和 TOTAL SIGNAL 信标增加透景玻璃边界、轻量 blur、文字阴影和窄屏单行约束，使动态插画上的实时信息稳定可读；错误态、forced-colors 与 reduced-motion 兜底保持。真实五档截图、几何和控制台证据见 `ui-accessibility-evidence.md` 的 v57 章节；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v58 自动采集连接状态

自动采集卡片的“未连接”徽标统一为低 alpha 玻璃状态点，并为 ready/error/forced-colors 保留状态边界；真实四档内容区几何、截图和控制台证据见 `ui-accessibility-evidence.md` 的 v58 章节。真实 Provider 联调、设备、辅助偏好和正式部署验收仍未关闭。

## v59 活动轨迹空态响应式修复

活动轨迹无事件时不再继承桌面 `540px` 数据表格画布；空态会在窄卡片内完整换行，真实事件仍保留表格滚动边界。390/768/1024/1440 隔离浏览器几何、截图和页面控制台证据见 `ui-accessibility-evidence.md` 的 v59 章节。真实非空事件、设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v60 分析图表空态密度修复

趋势与模型占比无数据时改为紧凑 signal checkpoint，连接区更早进入滚动路径；真实数据 ready 状态仍使用完整绘图区。320/390/768/1024/1440 隔离浏览器几何、截图和页面控制台证据见 `ui-accessibility-evidence.md` 的 v60 章节。真实非空图表、设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v61 顶部品牌栏透明优先

AI TOKEN 顶部品牌栏首屏恢复完全透景，滚动时只保留极轻的局部玻璃、细边线和低强度阴影；真实 320/390/768/1024/1440 计算样式、截图、无横向溢出和清洁控制台证据见 `ui-accessibility-evidence.md` 的 v61 章节。真实设备、reduced-motion、forced-colors、Provider 联调和正式部署验收仍未关闭。

## v62 短高度桌面首屏节奏

短高度桌面仅收敛 hero stage 底部留白，使周期切换器完整显示在第一屏；真实 1683×845 与 320/390/768/1024/1440 响应式几何、截图、无横向溢出和清洁控制台证据见 `ui-accessibility-evidence.md` 的 v62 章节。真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v63 Dashboard 场景首帧稳定性

首屏场景插画加入高优先级预加载、图片请求优先级和深蓝解码 fallback，消除首次导航时近黑空画布；冷 origin 的 768×900 首帧与 320/390/768/1024/1440 矩阵证据见 `ui-accessibility-evidence.md` 的 v63 章节。真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v64 透明顶栏文字锐化

顶栏品牌、导航和用户标签改用更紧的文字 keyline 与短 halo，在滚动插画上保持锐利而不填充背景；真实桌面首屏/Activity 滚动态、390px 截图和五档 computed matrix 证据见 `ui-accessibility-evidence.md` 的 v64 章节。真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v65 锚点导航受控过渡

同文档导航使用 420–760ms 可取消过渡，并在完成时按目标的 `scroll-margin-top` 重新校准；真实桌面 Activity、移动 auto-entry 和五档响应式证据见 `ui-accessibility-evidence.md` 的 v65 章节。reduced-motion 实际模拟、真实设备、Provider 联调和正式部署验收仍未关闭。

## v66 滚动状态透明顶栏

滚动态顶栏移除 `.045` 黑色 veil 与 `blur(4px)`，首屏和滚动状态均为完全透明，只保留底线、内侧发丝线和进度线；真实 390px/1440px 证据及五档响应式矩阵见 `ui-accessibility-evidence.md` 的 v66 章节。reduced-motion 实际模拟、真实设备、Provider 联调和正式部署验收仍未关闭。

## v67 基础透明首帧

基础 `.site-header` 现在从第一层 CSS 就使用 transparent/无 blur，不再依赖后加载视觉层覆盖黑玻璃；真实 390px/1440px 首屏与滚动态及五档响应式证据见 `ui-accessibility-evidence.md` 的 v67 章节。reduced-motion 实际模拟、真实设备、Provider 联调和正式部署验收仍未关闭。

## v68 认证焦点反馈

认证页自动聚焦现在保持 quiet edge，用户首次键盘/鼠标操作后恢复完整 `:focus-visible` halo，移动端不抢焦点；真实桌面/移动截图与五档响应式证据见 `ui-accessibility-evidence.md` 的 v68 章节。reduced-motion 实际模拟、真实设备、Provider 联调和正式部署验收仍未关闭。

## v69 透明品牌栏与窄视口焦点边界

AI TOKEN 顶栏默认态与滚动态现在完全透景，移除残留边线、内侧高光、背景图与阴影，仅保留文字可读性阴影和滚动进度线；自动聚焦仅在细指针桌面宽度启用。5146 隔离实例的桌面、移动和五档矩阵证据见 `ui-accessibility-evidence.md` 的 v69 章节。真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v70 移动滚动态安全顶栏

移动端首屏保持完全透明；滚动后使用低 alpha 安全渐变与短 blur，防止首屏 CTA 穿入 sticky brand/account 操作区；桌面仍保持透明顶栏。5148 隔离实例的真实移动/桌面证据见 `ui-accessibility-evidence.md` 的 v70 章节，真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v71 桌面滚动态安全顶栏

首屏继续使用完全透明顶栏；桌面滚动后增加深蓝低透明度渐变、12px blur 和轻阴影，防止 Dashboard 指标穿入品牌/导航行，同时保留背景插画透景。移动端维持较轻的独立 veil，forced-colors 由全断点系统配色规则接管。5150 隔离实例的 1440/390/320px 证据见 `ui-accessibility-evidence.md` 的 v71 章节，真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v72 透明顶栏文字对比度

首屏顶栏继续完全透明；非活动导航和用户名从低对比灰色提升至共享 `--ink-soft`，并使用短深色 keyline 保持亮色插画上的字形边缘，不改变导航结构或滚动安全层。5160 隔离实例的 1440/390/768/1024/1440、可访问性树和控制台证据见 `ui-accessibility-evidence.md` 的 v72 章节，真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v73 透明指标栏文字可读性

指标栏保持透明透景；序号、标题、数值与辅助说明增加局部字形 keyline，辅助说明提升至 `--ink-soft`，让亮色角色背景上的 token 统计仍可扫描。5170 隔离实例的 390/1440px 证据见 `ui-accessibility-evidence.md` 的 v73 章节，真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v74 reveal 过渡可读性

首屏以下内容的进入动效现在保留 `8px` 位移与 `.72s` 收束，但无障碍允许动效时预落位透明度不低于 `.62`，避免过渡帧把正文压到接近不可读；`prefers-reduced-motion: reduce` 仍走既有静止复位。5170 隔离实例的真实滚动、响应式、无障碍和控制台证据见 `ui-accessibility-evidence.md` 的 v74 章节；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v75 AI TOKEN 顶栏透明契约

AI TOKEN 顶栏在普通模式的首屏与滚动态现在统一完全透明，移除后加载样式造成的渐变、blur、阴影和底边线覆盖；导航文字 keyline、滚动进度线与 forced-colors 系统 Canvas 复位继续保留。5171 隔离实例的真实桌面/移动/响应式、无障碍和控制台证据见 `ui-accessibility-evidence.md` 的 v75 章节；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v76 Dashboard 深蓝玻璃面板

Dashboard 的图表、表单、记录和引导面板不再使用接近不透明的黑色渐变，普通配色下统一采用 `.84/.92` 深蓝玻璃层和 `16px` blur，让场景背景参与层级表达；`forced-colors: active` 不进入该规则，继续由系统 Canvas 接管。5172 隔离实例的真实响应式、无障碍和控制台证据见 `ui-accessibility-evidence.md` 的 v76 章节；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v77 AI TOKEN 顶栏场景透景

AI TOKEN 顶栏在普通模式继续完全透明；场景 veil 在桌面、平板和移动断点统一改为低 alpha 深蓝氛围层，移除造成黑色覆盖感的高 alpha 黑色横向渐变。5173 隔离实例已完成 390/320/768/1024/1440px 响应式、无障碍和控制台检查，独立证据见 `ui-accessibility-evidence-v77.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v78 认证自动聚焦舒适度

登录页自动聚焦现在使用低亮 lavender 边界，避免首帧同时叠加 lime outline、输入框 halo 和卡片光环；首次键盘/指针操作清除 marker 后仍恢复完整键盘焦点反馈。5174 隔离实例已完成 320/390/768/1024/1440px、Tab 行为、无障碍和控制台检查，独立证据见 `ui-accessibility-evidence-v78.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v79 Dashboard 滚动态导航安全层

Dashboard 在静止状态继续使用完全透明 AI TOKEN 顶栏；滚动后仅在 621px 以上的 nav/account 区域启用低 alpha 深蓝 reading rail，桌面与平板分别按真实几何覆盖导航，避免 CTA/表单内容穿透到操作层。5175 隔离实例已完成 320/390/768/1024/1440px 与 768/1440px 滚动态检查，独立证据见 `ui-accessibility-evidence-v79.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v80 空数据图表信号面

Dashboard 零数据图表现在使用低对比网格、零基线、等待扫描线和模型占比导引环，避免空账号看到大面积无意留白；现有空态文案与自动采集入口仍是主交互层。5176 隔离实例已完成 320/390/768/1024/1440px、周期按钮、可访问性快照和页面日志检查，独立证据见 `ui-accessibility-evidence-v80.md`；AI TOKEN 顶栏首屏与滚动态 computed background 继续为 transparent，真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v81 滚动态品牌阅读边界

Dashboard 在滚动态为 AI TOKEN 品牌锁定区增加 `.035` alpha 的局部透景保护镜，并让一次性的 `Scroll to explore` cue 离开 sticky header 阅读通道；整条顶栏仍保持 transparent/no global blur。5177 隔离实例已完成 320/390/768/1024/1440px 顶部矩阵、390/1440px 滚动态截图、周期按钮、可访问性快照和页面日志检查，独立证据见 `ui-accessibility-evidence-v81.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v82 指针跟随生命周期

Dashboard 的装饰性鼠标光环现在只在移动时出现，静止 1.8 秒后进入 `pointer-idle` 并停止 requestAnimationFrame，下一次移动可靠恢复；首帧隐藏、原生鼠标、hover/focus、reduced-motion 与 fine-pointer 边界继续保留。5178 隔离实例已完成指针状态序列、无横向溢出、Week/Today、无障碍快照和页面日志检查，独立证据见 `ui-accessibility-evidence-v82.md`；本轮是聚焦生命周期验证，四档 viewport、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v83 AI TOKEN 顶栏完全透景

滚动态不再绘制右侧 reading rail，也不再为 AI TOKEN 品牌锁定区绘制局部 lens；顶栏主体、品牌行和场景保持连续透景，仅保留文字 keyline 与滚动进度线。5179 隔离实例已完成默认桌面首屏/滚动态截图、computed style、Week/Today、无横向溢出、可访问性树和页面日志检查，独立证据见 `ui-accessibility-evidence-v83.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v84 TOTAL SIGNAL 内部扫描弧

Dashboard 的核心计数器增加低干扰 `core-sweep` 内部扫描弧，强化零数据状态下的 signal instrument 语义；动画仅作用于装饰伪元素，forced-colors 与 reduced-motion 边界明确关闭。5181 隔离实例已完成首屏截图、动画 computed style/时间采样、无横向溢出、可访问性语义和页面日志检查，独立证据见 `ui-accessibility-evidence-v84.md`；周期 pill 同步观察与 viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v85 周期 pill 同步

统计周期按钮在 pressed state 改变的同一任务内同步测量 active pill，并保留下一帧响应式校准，消除异步摘要刷新期间的旧位置残留。5181 隔离实例已完成 Today/Week/Month/All time 四档几何、状态文案、ARIA pressed、无横向溢出、顶栏透明回归和页面日志检查，独立证据见 `ui-accessibility-evidence-v85.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v86 滚动态透明顶栏阅读层

滚动态 AI TOKEN 顶栏继续保持完全透明填充、无背景图、无边线和无阴影，仅使用 `blur(10px) saturate(1.04)` 软化穿过 sticky chrome 的内容；首屏仍为无 blur 透景，`Scroll to explore` 在滚动后退出阅读通道。5182 隔离实例已完成首屏、720px、900px、1118px 滚动态截图与 computed style、标题安全落点、可访问性语义和页面日志检查，独立证据见 `ui-accessibility-evidence-v86.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v87 品牌区透景与 CTA 收束

滚动态不再对整条 AI TOKEN 顶栏做 blur：品牌区和 header 主体保持透明/无 blur，透明局部镜片只服务右侧导航与账户操作；离开首屏后 hero CTA 和 `Scroll to explore` 退出 sticky 阅读通道，键盘焦点中的 CTA、reduced-motion 和 forced-colors 保留独立边界。5183 隔离实例已完成首屏、720px、2220px 与返回顶部恢复的截图、computed style、焦点控件标签、无横向溢出和页面日志检查，独立证据见 `ui-accessibility-evidence-v87.md`；更广泛的深度滚动内容避让、viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v88 sticky 内容避让

深度滚动时，`sticky-occlusion.js` 只对进入透明 AI TOKEN 顶栏交叠带的非焦点 `.site-main .button` / `summary` 收束视觉层，保留布局盒和 Tab 顺序；`:focus-within` 恢复可见和可交互状态，hero actions 继续由 v87 规则负责。5184 隔离实例已完成首屏、720px、2220px、键盘焦点恢复、截图、无横向溢出和页面日志检查，独立证据见 `ui-accessibility-evidence-v88.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v89 预落位 reveal 可读性

允许动效下的 below-fold 内容预落位从 `.62 / 8px / .72s` 调整为 `.78 / 6px / .64s`，让自动采集卡片在刚进入视口时保持足够文字对比，同时保留轻量 editorial reveal；落位态、移动端静止规则和 reduced-motion 契约不变。5185 隔离实例已完成 `scrollY=720` 预落位、`scrollY=1200` settled、透明顶栏、无横向溢出和页面日志检查，独立证据见 `ui-accessibility-evidence-v89.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v90 周期切换器透景层

周期切换器在普通配色下改为 `.58` alpha 深蓝透景层与 `blur(12px) saturate(1.08)`，active lime pill、四档 period 状态和 forced-colors 边界保持不变。5186 隔离实例已完成首屏与滚动态截图、Today/Week/Month/All time 真实点击、ARIA pressed、无横向溢出和页面日志检查，独立证据见 `ui-accessibility-evidence-v90.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v91 sticky 标题阅读边界

sticky 避让模块现在同时保护按钮、折叠入口和 `.card-heading`：标题进入透明 AI TOKEN 顶栏交叠带时仅退出视觉层，保留 DOM 与辅助阅读；离开后恢复，包含按钮的管理标题通过 `:focus-within` 保持可操作。5187 隔离实例已完成 `scrollY=1118` 深度避让、`scrollY=898` 返回恢复、截图、无横向溢出和页面日志检查，独立证据见 `ui-accessibility-evidence-v91.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v92 自动采集操作行阅读边界

sticky 避让模块现在将自动采集 `.auto-form-actions` 与其提交按钮、隐私说明视为完整视觉单元；深度交叠时整行退出，提交按钮获得焦点时通过 `:focus-within` 恢复，离开后恢复正常显示。5188 隔离实例已完成 `scrollY=2200` 深度避让、Tab 焦点恢复、离开交叠带恢复、截图、无横向溢出和页面日志检查，独立证据见 `ui-accessibility-evidence-v92.md`；更上方表单字段残影、viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v93 透明品牌行表面边界

透明 AI TOKEN sticky 顶栏现在由 `sticky-occlusion.js` 同时测量内容表面交叠带；`responsive-tuning.css` 对卡片只裁剪顶栏覆盖的局部区域，让背景插画连续透过品牌行，不给顶栏重新加实心背景。`5189` 隔离实例已完成 `scrollY=2200` 顶栏透明计算样式、76px 交叠带裁切、备注字段 `:focus-within` 恢复、离开交叠带恢复、无横向溢出和页面日志检查，独立证据见 `ui-accessibility-evidence-v93.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v94 深滚孤立表面边界

当卡片在透明 AI TOKEN 顶栏下方只剩不超过 120px 的孤立下沿时，sticky 表现模块为其增加 `is-sticky-fragment`；非焦点状态整体退出，避免残片被误读为第二条顶栏，`:focus-within` 仍恢复完整卡片。`5190` 隔离实例已完成 `scrollY=2200` 完整过渡后的视觉截图、动作行隐藏、Tab 到备注字段并恢复、中间滚动主体可见、无横向溢出和页面日志检查，独立证据见 `ui-accessibility-evidence-v94.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v95 长页面 reveal 生命周期

`motion.js` 新增单帧 scroll/resize 同步器：当页面已经越过区块的 reveal 阈值时，立即补齐 `is-visible`、清除 transition delay 并解除观察，避免快速跳转漏掉 `IntersectionObserver` 回调后留下预落位灰度。5191 隔离实例已完成深度快跳约 `scrollY=2995` 与返回 `scrollY=1200` 的 8 区块 settled 检查、39 个交互控件命名、无横向溢出和清洁页面日志，独立证据见 `ui-accessibility-evidence-v95.md`；viewport override、真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v97 窄短手机首屏动作区

针对 `320×720` 窄短手机，响应式表现层仅收紧 hero stage 的上下留白、轨道尺寸、lede 间距和 footer gap，使“开始自动采集 / 查看分析”两个主操作完整落在第一屏；`320/360/390/768/1024/1440` 六档隔离矩阵均保持无横向溢出，独立证据见 `ui-accessibility-evidence-v97.md`。真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v98 指标栏局部透景层

指标栏三个 `.signal-cell` 现在使用低 alpha、无模糊的局部渐变和轻分隔线，数字在人物/设备插画上保持清晰，同时避免移动端形成连续深色横带；5194 隔离实例已完成 `320/390/768/1024/1440` 五档计算样式、截图、无横向溢出、控件命名和清洁日志检查，独立证据见 `ui-accessibility-evidence-v98.md`。真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v99 AI TOKEN 品牌行透明修正

最上方 AI TOKEN 品牌 header 在普通配色下明确保持 `transparent / no backdrop-filter / no box-shadow`；hero 元信息行取消贯穿全宽的底边线，改为局部 lime → lavender 短信号线，防止场景暗部被误读为黑色覆盖层。5195 隔离实例已完成 `320/390/768/1024/1440` 五档 computed style、移动滚动态、周期四档交互、无横向溢出和控件命名检查，独立证据见 `ui-accessibility-evidence-v99.md`。真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v100 顶部状态胶囊透明修正

hero 元信息右侧的 `#dashboard-status` 在普通配色下改为透明背景、无 blur 的状态边界，保留状态点、错误态颜色和轻量 keyline；`OBSERVATORY` 副标题增加局部字形阴影，保证透明品牌行在亮色插画上仍可扫描。5200 隔离实例已完成 `320/390/768/1024/1440` 五档 computed style、`scrollY=720` 滚动态、周期四档交互、无横向溢出和控件命名检查，独立证据见 `ui-accessibility-evidence-v100.md`。真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v101 登录认证卡片透景材质

登录认证卡片在普通配色下改为低 alpha 深蓝透景层与 `blur(14px) saturate(1.08)`，减少黑色大块感并让背景插画参与层次；字段对比、焦点边界、顶部高光和 forced-colors 复位保持不变。5201 隔离实例已完成 `320/390/768/1024/1440` 五档几何、无横向溢出、聚焦状态、控件命名和清洁日志检查，独立证据见 `ui-accessibility-evidence-v101.md`。真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v102 周期切换器局部透景

Dashboard 周期切换器在普通配色下从 `.58` alpha 深蓝玻璃与 `blur(12px)` 收敛为低 alpha 垂直渐变与 `backdrop-filter: none`，保持边界、active lime pill、ARIA pressed、键盘交互和 forced-colors 复位，避免顶部透明 AI TOKEN 品牌行与控件产生材质断层。5202 隔离实例已完成 `320/390/768/1024/1440` 五档无横向溢出、四档 period 点击、控件命名、标题层级、桌面/深滚截图和清洁页面日志检查，独立证据见 `ui-accessibility-evidence-v102.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v103 认证卡片局部透景

登录与注册认证卡片在普通配色下进一步收敛为 `.54/.66` alpha 深蓝渐变与 `blur(11px) saturate(1.06)`，让角色、代码屏和设备场景继续穿透卡片；输入字段的局部阅读底、焦点边界、顶部高光、forced-colors 与 reduced-motion 复位保持不变。5203 隔离实例已完成 `/login` 与 `/register` 的 `320/390/768/1024/1440` 五档无横向溢出、卡片 computed style、7/7 控件命名、焦点状态、桌面/移动截图和页面日志检查，独立证据见 `ui-accessibility-evidence-v103.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v104 Dashboard 内容卡片透景

Dashboard 图表、自动采集、说明、记录和手动明细卡片在普通配色下从 `.84/.92` alpha 与 `blur(16px)` 收敛为 `.70/.82` alpha 深蓝渐变与 `blur(12px) saturate(1.06)`，让滚动态继续保留插画的空间层次；表单字段局部阅读底、空态图表引导、焦点边界、forced-colors 与 reduced-motion 复位保持不变。5204 隔离实例已完成 `320/390/768/1024/1440` 五档无横向溢出、四档 period 点击、字段焦点、控件命名、标题层级、桌面/移动深滚截图和清洁页面日志检查，独立证据见 `ui-accessibility-evidence-v104.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v105 自动采集连接状态透景

自动采集标题旁的 `connection-badge` 在普通配色下改为低 alpha 垂直渐变、无 blur 的局部状态层，和透明 AI TOKEN 顶栏、周期切换器及 Dashboard 内容卡片使用同一层级语言；未连接、ready、error 的语义颜色与边界保留，forced-colors/reduced-motion 复位不变。5205 隔离实例已完成 `320/390/768/1024/1440` 五档无横向溢出、状态 computed style、Base URL 字段焦点、控件命名、标题层级、深滚截图和清洁页面日志检查，独立证据见 `ui-accessibility-evidence-v105.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v106 顶部元信息边界清理

`.hero-topline` 不再继承通用 surface 的顶部、左右边线，只保留局部 signal trace；因此透明 AI TOKEN 品牌行下方的场景不再被残留边框切成暗色横带。5206 隔离实例已完成 `320/390/768/1024/1440` 五档 computed style、周期交互、深滚层级、无横向溢出和清洁页面日志，独立证据见 `ui-accessibility-evidence-v106.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v107 首屏说明文案可读性

`.hero-lede` 在普通配色下增加紧凑的两级文字 keyline，解决移动端文案跨越笔记本与角色高光时的读字干扰；不增加背景面板，不改变透明 AI TOKEN 顶栏、hero 构图、DOM 或动效契约。5208 隔离实例已完成 `320×720/390×844/768/1024/1440` 五档 computed style、截图、周期交互、深滚、无横向溢出和清洁页面日志，独立证据见 `ui-accessibility-evidence-v107.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v108 认证辅助文案层级

登录/注册共享认证卡片的 helper copy 与 alternate-route link 在普通配色下提升到可读语义色，增加紧凑文字 keyline 与细 lime 下划线；不增加不透明遮罩，不改变认证流程、焦点顺序或动效降级。5209 隔离实例已完成登录/注册 `320/390/768/1440` 四档截图、computed style、用户名焦点、无横向溢出和清洁页面日志，独立证据见 `ui-accessibility-evidence-v108.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v109 周期控件 sticky 边界

Dashboard 周期切换器复用既有 `sticky-occlusion.js` 候选集合：当透明 AI TOKEN 顶栏覆盖控件时淡出并停止点击，焦点路径通过 `:focus-within` 恢复，未发生实际交叠的移动/桌面视口保持可见。5210 隔离实例已完成 `320/390/768/1024/1440` 五档几何、无横向溢出、焦点恢复和清洁页面日志，独立证据见 `ui-accessibility-evidence-v109.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v110 AI TOKEN 顶部品牌栏透光层

将顶部品牌栏统一为低 alpha 垂直透光渐变，关闭 `backdrop-filter` 与深色外投影，仅保留极轻的底部内侧高光；登录页与 Dashboard 首屏/深滚共用同一视觉契约，避免透明场景被模糊横带或黑色面板切断。5211 隔离实例已完成桌面、`390×844`、`320×720` 和 Dashboard 深滚验证，证据见 `ui-accessibility-evidence-v110.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。
## v111 认证卡片 signal line 呼吸

登录/注册卡片只在普通配色且允许动效时让既有顶部 signal line 追加低频光晕呼吸；卡片本体不参与 shadow 动画，`:focus-within` 保留原有键盘 focus 边界，forced-colors/reduced-motion 维持降级。5212 隔离实例完成桌面与 `390×844` 真实浏览器验证，证据见 `ui-accessibility-evidence-v111.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。
## v112 Dashboard 指标栏 sticky 避让

将三个 `.signal-cell` 纳入既有 `sticky-occlusion.js` 候选集合；当指标实际穿过透明 AI TOKEN 顶栏的 `76px` 阅读带时，复用统一的 `is-under-sticky-header` 规则淡出并停止指针命中，离开交叠区后恢复。焦点恢复、forced-colors 与 reduced-motion 边界沿用既有规则，不改变 Dashboard 数据、API 或布局契约。5213 隔离实例已完成桌面首屏与稳定 `scrollY=980` 深滚验证，证据见 `ui-accessibility-evidence-v112.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。
## v113 AI TOKEN 顶栏真正透明

普通配色下的 AI TOKEN 顶栏改为 `background: transparent`、`background-image: none`、无 `backdrop-filter`、无 `box-shadow` 的真实透景窗口；文字 keyline 与独立滚动进度线继续承担可读性和滚动态反馈，避免背景插画高光下出现浅色横带。5214 隔离实例完成登录页、Dashboard 首屏/深滚与 `320/390/768/1024/1440` 五档回归，证据见 `ui-accessibility-evidence-v113.md`；滚动进度线宽度异常作为下一轮独立观察项，真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。
## v114 异步高度下的滚动进度同步

共享导航模块使用 `ResizeObserver` 监听 `document.body` 尺寸变化，并复用既有 `requestAnimationFrame` 合帧，让图表加载、记录刷新和异常补录展开后自动重算阅读比例；品牌进度线由动态 `width` 改为固定宽度 `scaleX`，避免异步布局期间出现 `0px` 的过渡中间态。5215 隔离实例已完成真实展开交互、稳定深滚和 `320/390/768/1024/1440` 五档回归，证据见 `ui-accessibility-evidence-v114.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。
## v115 透明品牌行光学边界

新增 `header-chrome.css` 作为 01-shell 品牌 chrome 的高内聚样式模块：仅使用 1px composited hairline、品牌标记 hover/focus 反馈和文字 keyline，为真正透明的 AI TOKEN 顶栏补足空间边界；不增加背景填充、模糊、阴影、DOM、API 或业务状态。5216 隔离实例已完成 `390×844`、`1440×900` 登录首屏与 Dashboard 深滚验证，证据见 `ui-accessibility-evidence-v115.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。
## v116 Dashboard 空态观测舱

新增 `observatory-signal.css`，将无数据趋势图/模型占比面收敛为低 alpha 深蓝透景层、细坐标网格、`NO SIGNAL / READY` 标记和局部 signal line；周期切换器增加与 hero-foot 对齐的 hairline，不改变 Chart.js、接口、DOM 语义或 `aria-pressed`。5217 隔离实例已完成五档响应式、深滚空态和 Week 真实切换验证，证据见 `ui-accessibility-evidence-v116.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v117 AI TOKEN 品牌栏最终透明契约

新增 `brand-transparency.css` 并在共享 `base.html` 样式链末尾接入，普通配色下强制品牌栏及其子级保持透明、无背景图、无 blur、无 shadow，避免旧模块的后加载规则重新产生黑色横带；forced-colors 使用系统 Canvas 颜色，reduced-motion 关闭新增过渡。5218 隔离实例已完成 `320/390/768/1024/1440` 五档、首屏/滚动态和窄屏截图验证，证据见 `ui-accessibility-evidence-v117.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v118 认证字段 signal lane

新增 `auth-signal.css` 并在共享样式链末尾接入；登录/注册共用模板增加装饰性字段元信息和焦点局部 signal line，认证卡片收敛为低 alpha 深蓝透景层，移动端保留 50px 输入控件和 48px 主按钮。编号使用 `aria-hidden`，不改变 label 可访问名称、认证接口、CSRF、字段值或数据流。5219 隔离实例已完成 `320/390/768/1024/1440` 五档、登录/注册继承、真实焦点和清洁日志验证，证据见 `ui-accessibility-evidence-v118.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v119 自动采集协议 sticky rail

新增 `connect-rail.css` 并在共享样式链末尾接入；桌面 `901px+` 将 `HOW IT WORKS` 说明固定在自动采集长表单旁的 `96px` 阅读带内，触屏与窄桌面恢复普通文档流，保留既有 sticky occlusion、焦点恢复、forced-colors 和 reduced-motion 边界。5220 隔离实例已完成五档响应式、1440px 深滚、Base URL 焦点和 390px 长表单验证，证据见 `ui-accessibility-evidence-v119.md`；真实设备、辅助偏好、Provider 联调和正式部署验收仍未关闭。

## v120 顶部品牌场景兼容边界

基础 `style.css` 补回 `.story-backdrop` / `.story-backdrop-image` 的全屏背景兼容边界，修复旧个人入口只加载基础样式时透明 `.site-header` 下方落到黑色 body 画布的问题。当前完整模板加载的 `scene-motion.css` 继续覆盖同一选择器，品牌栏保持 `transparent / no backdrop-filter / no box-shadow`；真实 `5000` 与隔离 `5221` 已完成五档响应式、首屏截图、滚动态和清洁日志验证，证据见 `ui-accessibility-evidence-v120.md`。

## v121 移动认证入口节奏

基础 `style.css` 在 `620px` 以下、常规手机高度增加独立的认证入口节奏收敛：介绍区与认证卡片的间距、标题下方间距、说明段落间距和信任线行高略微降低，使主要输入更早进入视野；桌面双栏、认证契约、焦点和无障碍边界不变。真实 `5000` 已完成登录/注册五档、390px/768px/1440px 截图、键盘焦点和清洁日志验证，证据见 `ui-accessibility-evidence-v121.md`。
## 每次交付必须执行

从 `windows/` 目录执行：

```powershell
.\.venv\Scripts\python.exe -m compileall -q token_tracker
.\.venv\Scripts\python.exe -m token_tracker audit --json
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\packaging\toolchain-doctor.ps1
```

Android 和正式边缘门禁只能在对应环境执行；本地缺工具时保留 `pending`，不下载、不伪造
APK/EXE/Caddy 通过证据。所有变更必须回写 `tasks/todo.md`、必要 ADR 和本文件。

## v122 Dashboard 移动首屏节奏

基础 `style.css` 与 `responsive-tuning.css` 在 `620px` 以下统一收敛 Dashboard hero：orbit 使用移动视口上限，hero stage 取消多余最小高度，标题、主要入口与周期游标形成连续首屏阅读路径；不改变模板、周期交互、API、认证、Provider Key 或数据流。真实 `5000` 已完成 `320×720/320×844/390×844/768×900/1024×900/1440×900` 六档无横向溢出、390px 首屏几何、Week 周期交互、深滚空态和清洁日志验证，证据见 `ui-accessibility-evidence-v122.md`。

## 最终签署条件

1. Android APK 已由批准工具链构建、安装到授权设备并完成同账号联调。
2. EXE 已生成并验证用户数据目录、重启持久化、升级和回滚。
3. 真实 Provider usage 已完成脱敏非流式/流式证据；Key 未进入仓库、日志或数据库。
4. 合法认证会话下的 Web 五模块和管理员路径完成四档响应式、ARIA、焦点、错误态和导出证据。
5. HTTPS、备份恢复、限流、脱敏日志、ACL、轮转和回滚由部署负责人记录并复核。
6. 架构师检查依赖方向、契约、ADR、注释、行数和 Git 交付状态后，才可签署完成。
