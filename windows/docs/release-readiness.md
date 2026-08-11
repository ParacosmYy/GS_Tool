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
