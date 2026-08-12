# 六角色集成复核记录

**作者：** AI Token Tracker Engineering Team  
**复核对象：** AI Token Tracker 当前 checkout  
**复核日期：** 2026-08-11
**方法：** 代码边界审查、Python/JavaScript 编译诊断、本地 HTTP、真实浏览器 DOM/截图/控制台复核；未创建或运行测试专用资产。

## 复核结果

| 角色 | 关注点 | 结果 | 证据与遗留风险 |
|---|---|---|---|
| UI-1 视觉系统 | Material 3 语义 token、Moonshot-inspired 黑底大排版、首屏/图表/表单/空状态一致性 | 通过 | `docs/design-tokens.md`、`style.css`、`ui-polish.css`、`scene-motion.css`；v14 御姐风嵌入式工程师、MacBook Pro/Mac Studio 风格工作站、Rust/RL 屏幕语义、显式图像场景层、认证玻璃卡片和可读性遮罩已落地；v28 补充半透明玻璃层、独立表单表面、focus-within 和信号轨道降级，v29 修正左上场景遮罩并完成导航玻璃层运行观察，v30 补充图表空态行动入口和非空回归，v31 修复登录首帧黑幕并收紧 Admin 数据轨道，v34 收紧自动采集说明卡片表面高度并消除空黑区域，v36 统一三类表格零信号空态并补充信标层次，v38 统一工作信号与自动采集 select 的暗色玻璃表面，v39 降低顶部 AI TOKEN 品牌栏深色遮罩并保留透景可读性，v40 完成异常补录 disclosure 的闭合/展开表面状态，v42 收紧图表/活动/历史数据面阅读遮罩并保留顶部透明导航，v43 将品牌栏背景归零并强化历史账本的模型/总量/来源层级，v45 将滚动态品牌栏收敛为透明羽化渐变；v13/v12/v11/v10/v9/v8 仍保留为回滚资产；合成边界见 ADR-057/065/070/074/076/080/086/090/095。 |
| UI-2 动效交互 | reveal、轨道、scanline、glitch、count-up、pointer follower、异步状态 | 通过 | `docs/motion-contract.md`、`static/modules/motion.js`、`static/modules/navigation.js`、`static/modules/range-switcher.js`、Android `ui/MotionPreferences.kt`；Web pointer follower 只在移动后按需 requestAnimationFrame，Web reduced-motion 与 Android 系统动画缩放均不启动对应无限装饰循环；Dashboard、认证页与 Admin 共享 pointer/backdrop/reveal/surface motion，初始化隐藏的动态详情面板不会保持低透明度，导航 active 状态同时使用可见高亮和 `aria-current`，v34 仅调整自动采集双栏表面对齐，v35 提高待显区块可读性并保持进入视口后的完整收束，v36 为表格零信号空态增加同契约的 pulse/scan 微动效，v37 为周期切换器增加连续 active pill 与 pressed 状态同步，v40 为异常补录 disclosure 增加顶部信号线、open 状态过渡和内容渐入，并保留 reduced-motion 降级，v41 为章节导航补充 sticky header 安全落点，v42 仅调整数据面板遮罩，不改变默认平滑滚动节奏，v43 仅调整透明品牌层和账本排版，不增加装饰动画，v45 仅调整滚动态背景层，不新增监听器或无限动画，详见 ADR-079 与 v32/v33/v34/v35/v36/v37/v40/v41/v42/v43/v45 运行证据。 |
| UI-3 响应式与可访问性 | 语义标题、label、live region、表格 caption、focus-visible、空/错误状态 | 条件通过 | 登录/注册页与受保护 Dashboard/Admin 已完成隔离合法会话下 320/768/1024/1440 viewport、横向溢出、焦点回流、ARIA、动态错误播报、空态、历史非空、脱敏日志详情、服务端 CSV 200、Provider 502 错误态；认证页 v23 已将唯一语义 h1 与 aria-hidden glitch 视觉层分离；个人 Activity 表单/最近表格已完成 v19 隔离会话下成功、校验错误、恢复、锚点留白和默认视口无溢出证据；v26 补充 v14 默认浏览器首屏，v27 补齐当前 v14 320/768/1024/1440 viewport、唯一 h1、焦点、文字颜色和清洁控制台证据；v35 补齐待显区块的可读性、锚点进入视口后的完整收束和 320px 无溢出；v39 补充顶部品牌栏滚动保持、移动端导航收敛与无溢出证据，v40 补充原生 details 的 Space/Enter 键盘交互、summary 焦点和移动端无溢出证据，v41 补充 Analysis/Connect/Activity 章节标题安全落点、History 底部边界和移动端无溢出证据，v42 补充非空/空态图表、数据面板对比度和 320px 无溢出证据，v43 补充历史账本 320px 卡片化、空态回归、来源语义和透明品牌栏源码边界，v45 补充透明羽化滚动态品牌栏、移除背景 blur、桌面/320px 无溢出和清洁控制台证据；真实设备指标、浏览器下载落盘、真实 Provider 成功、reduced-motion/高对比度真实环境仍待部署级复核，详见 `docs/ui-accessibility-evidence.md`。 |
| 前端工程师 | API client、CSRF、XSS 边界、Key 生命周期、状态编排、静态 UI 契约 | 通过 | `app.js` 仅编排；API/图表/motion 已拆模块；渲染记录统一使用 `textContent`；Key 仅页面内存；`token_tracker audit` 已检查 label、table caption/scope、图片 alt、跳过链接和动效降级。 |
| 后端工程师 | 鉴权、用户隔离、参数化 SQL、SSRF、provider usage、响应上限、限流、日志与缓存边界 | 通过（源码边界）；真实 provider 联调待授权 | `web.py`/`providers.py`/`provider_projection.py`/`ingest_auth.py`/`gateway.py`/`gateway_reporting.py`/`gateway_contracts.py`/`gateway_queue.py`/`backup.py`/`db.py`/`events.py`/`csv_export.py`；health 200、未登录 summary 401、统一错误包络、安全响应头、凭据脱敏、provider 原始响应投影、Usage Ingest digest/到期/撤销/固定 source、Gateway loopback/独立访问令牌、上游 URL/大小/超时、JSON/SSE usage 解析、有界退避、DPAPI 跨重启恢复、响应读取异常映射、幂等键控制字符校验、非回环 HTTP 拒绝、队列元数据 fail-closed 和运行时队列故障熔断、动态中心端口只读发现、备份清单只读策略和完整性边界已通过源码与隔离 smoke，真实 provider Key 联调仍待授权，详见 ADR-037/039/040/052/053/054/056/058/059/060/068/075/077。个人入口 host 解析由架构边界收敛，详见 ADR-078。 |
| 架构师 | 依赖方向、API/ADR、六角色边界、根入口、版本控制、依赖锁定和演进风险 | 通过（源码边界）；正式环境门禁进行中 | `docs/architecture.md`、`docs/api-contract.md`、ADR-003/032/033/034/035/036/037/038/039/040/041/042/043/044/045/046/047/048/049/050/051/052/053/054/055/056/057/058/059/060/061/062/063/064/065/066/067/068/069/070/071/072/073/074/075/076/077/078/079/080/081/082/083/084/085/086/087/088/089/090/091/092/093/094/095/096/097/098/099、`roles/`、`start.bat`/`run.py`；访问日志、Caddy 边缘预检、敏感 artifact 隔离、本地 Git 保存点、provider 响应投影、runtime lock、统一有界 CSV、代码/文本 1000 行门禁、production/Caddy/LAN 部署契约、Android Release HTTPS 门禁、liveness/readiness 探针、跨端 v14 资产、稳定场景主图层、场景可见度构图、Web UI 静态契约、Kimi provider 预设、Kimi 官方 endpoint 上下文与 Key 隔离、EXE 构建工具链锁定、只读发布审计、本地 Chart.js 供应链、Usage Ingest、Gateway 契约、响应可靠性、DPAPI 重试队列完整性、运行时队列故障边界、备份 schema/身份完整性、只读备份清单与保留策略边界、生产 loopback 绑定、动态 live-region 契约、分享 handoff 预检、个人 Activity 写入边界、动态中心发现、个人入口 host fail-closed、CLI Waitress/显式共享绑定、签名 fail-closed、Android 系统动效策略、用户授权 SDK 入口和 LAN/示例 Production 预检证据均有记录；Gateway 真实上游联调、正式 HTTPS/Android/签名/正式分发/真实设备和真实备份责任人仍是门禁。 |

## v44 视觉与动效增量

- UI-1：首屏 header 保持完全透明，滚动态玻璃层从 `scene-motion.css` 统一承载，避免继续膨胀 `ui-polish.css`。
- UI-2：滚动状态通过既有 requestAnimationFrame 进度调度更新，不新增独立监听器或无限动画；reduced-motion 继续禁用过渡。
- UI-3：登录页与 Dashboard 的 DOM/ARIA 结构不变，隔离浏览器确认桌面/320px 无横向溢出和本地控制台清洁。

## v45 视觉与动效增量

- UI-1：滚动态 `.site-header.is-scrolled` 改为透明羽化渐变，不再使用深色实心背景；同时移除滚动态背景 blur，避免白色 hero 标题在品牌栏内形成发白雾带。
- UI-2：继续复用 `navigation.js` 的 requestAnimationFrame 滚动状态，不增加监听器、无限动画或页面脚本分支；现有 reduced-motion 降级边界保持不变。
- UI-3：隔离实例确认桌面 `1683×892` 与移动 `320×800` 的首屏/滚动态 computed style、无横向溢出和本地控制台清洁，DOM/ARIA 结构不变。

## v46 视觉与动效增量

- UI-1：不改变现有视觉层，只扩展锚点安全落点，保证主 CTA 进入自动采集卡片后仍保留标题、字段和 sticky header 之间的呼吸空间。
- UI-2：复用原有 hash 平滑滚动与 motion 状态，不新增滚动监听器、计时器或装饰动画。
- UI-3：隔离浏览器确认桌面/320px CTA 深链接、补录深链接、header 几何、无横向溢出和本地控制台清洁。

## v47 视觉与动效增量

- UI-1：基础态 AI TOKEN 品牌栏保持真正透明，移除基础态背景 blur 与外部阴影；仅保留细边线、内侧高光和文字阴影，滚动态羽化层继续由 `scene-motion.css` 统一承载。
- UI-2：本轮仅改 CSS，不新增滚动监听器、计时器或无限动画；现有 `prefers-reduced-motion` 降级边界保持不变。
- UI-3：隔离浏览器确认桌面/320px 首屏与滚动态 computed style、截图、无横向溢出和本地控制台清洁，DOM/ARIA 结构不变。

## v48 视觉与动效增量

- UI-1：登录/注册共享卡片降低遮罩 alpha 与背景 blur，让场景透景成为真实层次；高对比度与 forced-colors 仍切换到清晰不透明面。
- UI-2：本轮仅调整表面 CSS 参数，不新增监听器、计时器、依赖或动画；既有 focus、reduced-motion 和键盘路径保持不变。
- UI-3：隔离浏览器确认登录页 320/768/1024/1440 四档卡片几何、注册页复用、无横向溢出和本地控制台清洁。

## v49 视觉与动效增量

- UI-1：将交互态 pointer aura/ring 从大面积高亮收敛为更轻的 `300px/.68` 与 `48px/.92`，避免覆盖登录字段的原生焦点边界；基础跟随、surface spotlight 和按钮磁吸保持。
- UI-2：本轮仅调整 `scene-motion.css` 参数与注释，不新增监听器、计时器、依赖或业务分支；fine-pointer、reduced-motion 和 touch 降级保持不变。
- UI-3：隔离浏览器确认登录页 pointer-ready/interactive、surface CSS 变量、按钮磁吸、交互态/离开态回收和清洁控制台。

## v50 视觉与动效增量

- UI-1：滚动状态的 AI TOKEN 品牌栏保持透景语义，增加有限 alpha 的渐隐层和 `10px` 背景模糊，解决 sticky header 下方统计标签穿透造成的阅读干扰；首屏透明状态不变。
- UI-2：本轮只修改 `scene-motion.css` 的滚动态 CSS 与企业级注释，不新增 JS 监听器、依赖、DOM 或业务分支；键盘焦点、reduced-motion、forced-colors 和 touch 门控继续复用原实现。
- UI-3：隔离 Dashboard 真实浏览器确认首屏截图、滚动态截图、透明/半透明 computed style、桌面无横向溢出、skip-link 键盘焦点和清洁控制台；移动矩阵不在本轮重复宣称。

## v51 视觉与动效增量

- UI-1：为 Analysis/Activity/History 等 hash 目标消除文档顺序 reveal 的额外等待，保留短时 `.56s` 收束；长距离平滑滚动负责空间移动，目标内容不再在落点后继续保持低透明度。
- UI-2：本轮只增加 `scene-motion.css` 的 `:target` 表现规则，不改 JS、DOM、业务状态或依赖；普通滚动 reveal、reduced-motion 和键盘焦点边界保持原实现。
- UI-3：隔离 Dashboard 真实浏览器确认 Activity/Analysis 深链接的目标 opacity、transform、delay、duration、header 落点、桌面无溢出、skip-link 焦点和清洁控制台；移动矩阵未在本轮重复宣称。

## v52 视觉与动效增量

- UI-1：移动端 hero CTA 从可变 flex 换行收敛为 320px 下的两列等宽入口，保证主要自动采集与分析路径在首屏同一视觉扫描层内可见；桌面和平板保留原 flex 节奏。
- UI-2：本轮只调整 `style.css` 的窄屏表现规则，不新增 DOM、JS、依赖或业务分支；按钮原生键盘焦点、pointer/reduced-motion 和触摸降级继续复用既有实现。
- UI-3：隔离浏览器确认 320px、768px、默认桌面三档 CTA 几何、无横向溢出、skip-link 焦点和清洁控制台；1024/1440 与真实设备不在本轮重复宣称。

## v53 视觉与动效增量

- UI-1：AI TOKEN 顶部品牌栏在首屏和滚动态均保持透景透明，去除造成黑色横条观感的渐隐 veil 与 blur，仅保留细边线、文字阴影和滚动进度线。
- UI-2：本轮仅调整 `scene-motion.css` 的 header 装饰层规则，不新增 DOM、JS、依赖或业务分支；导航、锚点、键盘焦点、reduced-motion 与 forced-colors 边界保持。
- UI-3：隔离 Dashboard 真实浏览器确认 `scrollY=0` 和 `scrollY=1430` 两种状态的 computed style 与截图，控制台无 error/warning；未将本轮桌面结果扩大为完整响应式或设备验收。

## v54 视觉与动效增量

- UI-1：为 621–900px 平板单列 hero 释放垂直预算，保持轨道为视觉锚点，同时将主 CTA 提前到 768/900px 首屏；320px 双列 CTA 与 1024px 以上桌面构图保持。
- UI-2：新增独立 `responsive-tuning.css` 并由 `base.html` 按最后一层加载；只包含断点 CSS，不改 DOM、JS、依赖、业务状态或动画控制，文件行数远低于 1000 行。
- UI-3：真实浏览器确认 320/768/900/1024/1440 五档 CTA 几何、无横向溢出、skip-link 焦点和清洁控制台；真实设备及辅助偏好仍不宣称通过。

## v55 视觉与动效增量

- UI-1：顶部 AI TOKEN 品牌栏由完全透明但缺少层次的状态收敛为低 alpha 玻璃膜，保留插画透景，使用受控渐变、轻微 blur、顶部高光和滚动态阴影改善文字扫描性；不改品牌 DOM、导航和滚动进度线。
- UI-2：手机认证入口通过 flex `order` 将表单卡片提到介绍区前，平板通过 grid row 保持表单优先，桌面双栏继续沿用既有列映射；修改集中在 `scene-motion.css` 与 `responsive-tuning.css`，并补齐后加载场景层的 forced-colors 复位，无业务分支、依赖或数据契约变化。
- UI-3：真实浏览器复核 390/768/1024/1440 的登录/注册按钮几何、顶栏初始/滚动态 computed style、截图、无横向溢出与清洁控制台；真实设备、辅助偏好及正式部署仍不宣称通过。

## v56 视觉与动效增量

- UI-1：移动 Dashboard 仍保留轨道作为首屏视觉锚点，但将 320/390px 的轨道尺寸与文案间距收敛，主 CTA 从底边前移并恢复呼吸空间；768px 以上不改变既有 hero 构图。
- UI-2：增量仅落在 `responsive-tuning.css` 的移动断点，不新增 DOM、JS、依赖或业务状态；既有按钮焦点、导航语义、pointer/reduced-motion 和触摸降级继续复用。
- UI-3：真实浏览器确认 320/390/768/1024/1440 的轨道和 CTA 几何、截图、无横向溢出及清洁控制台；真实设备、辅助偏好与正式部署仍不宣称通过。

## v57 视觉与动效增量

- UI-1：为 Dashboard `#dashboard-status` 与 `TOTAL SIGNAL` live instrument 增加低 alpha 玻璃信息层、可控文字阴影和信号点，提升插画背景上的可读层级，不改变数据内容。
- UI-2：窄屏状态胶囊在 360px 以下收敛 padding、字号和字距并保持单行；错误状态、forced-colors 和既有 reduced-motion 边界均保留，未新增脚本或依赖。
- UI-3：真实浏览器确认 320/390/768/1024/1440 的状态层、总量信标、CTA 几何、截图、无横向溢出与清洁控制台；真实设备、辅助偏好及正式部署仍不宣称通过。

## v58 视觉与动效增量

- UI-1：自动采集卡片的 provider connection badge 采用与 Dashboard 状态胶囊一致的玻璃信息语法，未连接、ready、error 三态均有清晰边界，不改变状态文本或服务逻辑。
- UI-2：修改集中在 `scene-motion.css`，补齐 forced-colors 复位，未新增 DOM、JS、依赖或数据契约；原生焦点、reduced-motion 和移动触摸降级继续复用。
- UI-3：无缓存真实浏览器确认 390/768/1024/1440 的徽标几何、自动采集/指南内容区无横向溢出、截图与清洁控制台；真实 provider 和设备验收仍不宣称通过。

## v59 视觉与动效增量

- UI-1：将活动轨迹的空态从桌面数据网格中语义分离；窄卡片不再被 `540px` 最小宽度强迫横向滚动，空提示改为容器内可换行的阅读卡片。
- UI-2：状态切换集中在 `activity.js` 的 `is-empty` 视图状态，响应式表现集中在 `responsive-tuning.css`；有真实事件时仍沿用原表格宽度和横向浏览能力，未复制业务逻辑或数据契约。
- UI-3：真实隔离浏览器确认 390/768/1024/1440 的空态几何、页面与活动卡片无横向溢出、完整提示语截图和清洁页面控制台；真实非空事件、设备、辅助偏好和 Provider 联调仍不宣称通过。

## v60 视觉与动效增量

- UI-1：将无数据趋势/模型占比从大面积黑色空画布收敛为紧凑 signal checkpoint，保留加号轨道、状态解释和自动采集 CTA，缩短首次使用者到连接区的视觉距离。
- UI-2：由 `charts.js` 显式维护 chart card 的 `is-empty/is-unavailable` 状态，响应式层只作用于无数据状态；ready 图表的高度、Chart.js 交互和数据契约不变，未新增依赖或复制图表逻辑。
- UI-3：真实隔离浏览器确认 320/390/768/1024/1440 的空态高度、文案/CTA 可见性、页面无横向溢出、截图和清洁页面控制台；真实非空图表、设备、辅助偏好和 Provider 联调仍不宣称通过。

## v61 视觉与动效增量

- UI-1：AI TOKEN 顶部品牌栏采用透明优先策略；首屏完全透景，滚动仅叠加约 4px 的局部轻玻璃与发丝底线，移除造成整条黑色横带的高 alpha 渐变与重 blur。
- UI-2：变更集中在 `scene-motion.css` 的装饰层，不新增 DOM、JS、依赖、业务状态或数据契约；导航语义、键盘焦点、reduced-motion 与 forced-colors 既有边界保持。
- UI-3：真实浏览器确认 320/390/768/1024/1440 的首屏透明 computed style、滚动态轻玻璃、桌面/手机截图、无横向溢出与清洁页面控制台；真实设备、辅助偏好、Provider 联调和正式部署仍不宣称通过。

## v62 视觉与动效增量

- UI-1：针对短高度桌面首屏回收 hero stage 底部留白，使周期切换器完整进入第一屏；不缩小标题、轨道或主 CTA，不改变移动/平板的自然滚动节奏。
- UI-2：变更仅落在 `responsive-tuning.css` 的 `901px+ / 860px-` 断点，无 DOM、JS、依赖、数据契约或动画状态变化；既有 reduced-motion、forced-colors 和键盘焦点边界保持。
- UI-3：真实浏览器确认 1683×845、320/390/768/1024/1440 的周期切换器几何、截图、无横向溢出与清洁页面控制台；真实设备、辅助偏好、Provider 联调和正式部署仍不宣称通过。

## v63 视觉与动效增量

- UI-1：解决场景插画解码间隙造成的首帧近黑空画布；首屏资源提前请求，加载期间使用深蓝 fallback，保持 Dashboard 视觉连续性。
- UI-2：变更集中在 `base.html` 的资源提示/图片优先级和 `scene-motion.css` 的背景 fallback，不改变业务 DOM 语义、API、数据契约或交互状态；既有 reduced-motion、forced-colors 和键盘焦点边界保持。
- UI-3：冷 origin 真实 768×900 首帧截图、320/390/768/1024/1440 图片完成矩阵、无横向溢出与清洁控制台均已确认；真实设备、辅助偏好、Provider 联调和正式部署仍不宣称通过。

## v64 视觉与动效增量

- UI-1：将透明顶栏的品牌、导航和用户标签从宽泛发光改为紧 keyline + 短 halo，提升滚动插画上的字形锐度，不引入实心黑色横条。
- UI-2：变更仅落在 `ui-polish.css` 的共享文字表现层，不改变 DOM、JS、API、数据契约或交互状态；既有 reduced-motion、forced-colors、键盘焦点和透明 header 规则保持。
- UI-3：真实 1440×900 首屏/Activity 滚动态、390px 移动截图及 320/390/768/1024/1440 computed matrix 均已确认；真实设备、辅助偏好、Provider 联调和正式部署仍不宣称通过。

## v65 视觉与动效增量

- UI-1：将长距离锚点跳转从浏览器不可控的 distance-based smooth scroll 收敛为 420–760ms 受控过渡，避免用户等待数秒并让目标 section 稳定落在透明 header 下方。
- UI-2：导航行为集中在 `modules/navigation.js`，保留同文档 hash、ARIA active 状态、键盘/触摸语义和 reduced-motion 直达路径；不改变业务 DOM、API、数据契约或表单状态。
- UI-3：真实 1440px Activity、390px auto-entry 锚点与 320/390/768/1024/1440 响应式矩阵已确认落点、无横向溢出和清洁控制台；reduced-motion 实际设备模拟、Provider 联调和正式部署仍不宣称通过。

## v66 透明顶栏滚动态增量

- UI-1：移除滚动态 header 的半透明黑玻璃与 blur，使 AI TOKEN 品牌栏在首屏和滚动后保持一致透景；仅保留发丝边界、文字 halo 与进度线维持层级和可扫描性。
- UI-2：变更集中在 `scene-motion.css` 的 `.site-header.is-scrolled` 表现规则，不改变 DOM、JS、API、数据契约或表单状态；forced-colors 仍由专用规则接管。
- UI-3：真实 390px/1440px 滚动态 computed style、截图及 320/390/768/1024/1440 矩阵确认完全透明和无横向溢出；reduced-motion 实际设备、Provider 联调和正式部署仍不宣称通过。

## v67 基础样式层级增量

- UI-1：将 `.site-header` 的透明策略下沉到基础 `style.css`，消除默认黑玻璃先绘制、后被主题层覆盖的首帧风险；滚动态仍保持透景和轻量边界。
- UI-2：变更只触及基础顶栏表现声明，不改变 DOM、JS、API、数据契约、表单状态或强制配色例外；无新增依赖和公共交互分支。
- UI-3：真实 390px/1440px 首屏与滚动态 computed style、截图及 320/390/768/1024/1440 矩阵确认透明、无 blur、无横向溢出和清洁页面日志；真实辅助偏好、设备、Provider 联调和正式部署仍不宣称通过。

## v68 认证焦点反馈增量

- UI-1：将自动聚焦从首屏强荧光报警态收敛为 quiet edge，首次用户键盘/鼠标操作后恢复完整 `:focus-visible` halo；移动端继续不自动抢焦点。
- UI-2：状态边界集中在 `auth.js` 的 `data-auth-autofocus` 标记和独立 `auth-focus.css` 的表现规则，不改变表单字段、提交契约、认证逻辑或 API；无新增依赖。
- UI-3：真实桌面首屏与 Tab 键焦点截图、390px 移动截图、五档响应式矩阵和清洁页面日志均已确认；真实辅助偏好、设备、Provider 联调和正式部署仍不宣称通过。

## v69 透明品牌栏与窄视口焦点边界

- UI-1：顶部 AI TOKEN 品牌栏默认态与滚动态均移除背景图、内侧高光、底部边线与阴影，插画直接透景；滚动进度线继续承担状态反馈，文字 keyline 保持可读性。
- UI-2：表现规则集中在 `ui-polish.css` 与 `scene-motion.css`，自动聚焦边界集中在 `auth.js` 与 `auth-focus.css`；未改变 DOM、API、认证契约、数据流或业务状态。
- UI-3：真实 5146 隔离实例已完成桌面首屏、PageDown 滚动态、移动首屏与 320/390/768/1024/1440 矩阵；真实设备、辅助偏好、Provider 联调和正式部署仍不宣称通过。

## v70 移动滚动态安全顶栏

- UI-1：移动端静止首屏继续使用完全透明品牌栏；滚动态仅启用低 alpha 渐变、短 blur 和轻阴影，避免首屏 CTA 滑入 AI TOKEN/Log out 的操作安全区；桌面透明策略保持不变。
- UI-2：变更集中在 `responsive-tuning.css` 的移动断点，并显式补齐 forced-colors 系统配色复位；未改变 DOM、导航契约、业务状态、API 或数据流。
- UI-3：真实 5148 实例已完成 390px 首屏/scrollY≈700、1440px 滚动态和清洁页面观察；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署仍不宣称通过。

## v71 桌面滚动态安全顶栏

- UI-1：保持首屏透明优先，只在滚动态为所有断点提供深蓝低透明度 reading veil；移动端继续使用较轻的局部 veil，避免统计指标穿入 sticky 导航，同时保留场景透景。
- UI-2：表现变化集中在 `responsive-tuning.css`，仅覆盖 `.site-header.is-scrolled` 的视觉状态，并把 forced-colors 复位提升为全断点系统配色规则；未改变 DOM、导航契约、业务状态、API 或数据流。
- UI-3：真实 5150 隔离实例已完成 1440/390/320px 首屏与滚动态 computed style、截图、无横向溢出和清洁页面日志确认；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署仍不宣称通过。

## v72 透明顶栏文字对比度

- UI-1：保持顶栏完全透明与场景连续透景，将非活动导航和用户名提升至 `--ink-soft`，以紧凑深色 keyline 提升亮色插画上的扫描性，不引入装饰性实心条。
- UI-2：变更集中在 `responsive-tuning.css` 的共享顶栏文字表现层；未改变 DOM、导航契约、业务状态、API、数据流或滚动安全层。
- UI-3：真实 5160 隔离实例已完成 1440/390/768/1024/1440 矩阵、390px 滚动态、可访问性树、截图和清洁页面日志确认；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署仍不宣称通过。

## v73 透明指标栏文字可读性

- UI-1：指标栏继续完全透景，仅为跨越亮色插画的序号、标题、数值和辅助说明增加局部字形 keyline，并提升辅助说明色值，不改变信息层级。
- UI-2：变更集中在 `responsive-tuning.css` 的共享指标表现层；未改变 DOM、数据契约、图表逻辑、导航或业务状态。
- UI-3：真实 5170 隔离实例已完成 390/1440px 截图、滚动态 computed style、页面宽度和清洁页面日志确认；真实设备、reduced-motion、forced-colors、Provider 联调和正式部署仍不宣称通过。

## v74 reveal 过渡可读性

- UI-1：保留现有由下向上的 reveal 叙事，只把动效中的可读性地板从 `.34` 提升至 `.62`，并将位移/时长收敛为 `8px/.72s`；用户在等待落位时仍可读取标题、统计和操作文案。
- UI-2：变更只位于 `responsive-tuning.css` 的 `prefers-reduced-motion: no-preference` 表现层；`reduce` 复位、DOM、API、数据契约、导航和业务状态均未改变，未新增依赖。
- UI-3：真实 5170 实例已完成 1440px 滚动过渡、320/390/768/1024/1440 矩阵、无障碍快照和页面日志检查；真实 reduced-motion/forced-colors、设备、Provider 联调和正式部署仍不宣称通过。

## v75 AI TOKEN 顶栏透明契约

- UI-1：修正后加载样式覆盖关系，使普通模式下 AI TOKEN 品牌行在首屏和滚动态都直接透景；滚动进度线、文字 keyline 和 forced-colors 系统复位保持各自职责。
- UI-2：变更集中在 `responsive-tuning.css` 的共享顶栏状态层，删除深蓝渐变/blur/阴影覆盖，不改变 DOM、导航契约、业务状态、API 或数据流；移动断点不再引入第二套顶栏 veil。
- UI-3：真实 5171 实例已完成 1440/390/320/768/1024px 首屏与滚动态 computed style、无横向溢出、无障碍快照和页面日志检查；真实 reduced-motion/forced-colors、设备、Provider 联调和正式部署仍不宣称通过。

## v76 Dashboard 深蓝玻璃面板

- UI-1：将内容面板从接近实心黑色改为深蓝玻璃层，让固定场景在图表、表单和记录卡之间保持可见的纵深；不改变透明 AI TOKEN 顶栏或数据可读性契约。
- UI-2：变更集中在 `responsive-tuning.css` 的普通配色媒体分支，复用现有 surface 变量、边框和文字 token；不改变 DOM、API、数据流、表单行为或依赖。
- UI-3：真实 5172 实例已完成 390/320/768/1024/1440px 面板 computed style、无横向溢出、无障碍快照和页面日志检查；`forced-colors` 仅完成源码边界保护，真实辅助偏好、设备、Provider 联调和正式部署仍不宣称通过。

## v77 AI TOKEN 顶栏场景透景

- UI-1：保留 AI TOKEN 顶栏的 `transparent / no blur / no shadow` 契约；将场景 `::before` 在桌面、平板和移动断点的黑色 veil 改为低 alpha 深蓝氛围层，避免顶部与左半区呈现黑色覆盖块。
- UI-2：变更仅位于 `scene-motion.css` 的装饰层，不改变 DOM、导航、API、数据流、认证、Provider 或交互状态；移动断点与桌面断点拥有明确的局部 alpha 约束。
- UI-3：真实 5173 隔离实例已完成 390/320/768/1024/1440px computed-style、无横向溢出、可访问性快照和清洁页面日志检查；证据独立记录于 `ui-accessibility-evidence-v77.md`。

## v78 认证自动聚焦舒适度

- UI-1：将首次自动聚焦从荧光绿报警态收敛为单层 lavender keyline；用户发生实际键盘/指针操作后，恢复完整 `:focus-visible` 键盘焦点环。
- UI-2：变更集中在 `auth-focus.css`，通过现有 `data-auth-autofocus` 状态边界处理表现层，不改变认证 DOM、提交契约、会话、API、数据流或 forced-colors 系统复位。
- UI-3：真实 5174 隔离实例已完成 320/390/768/1024/1440px computed-style、桌面/移动截图、Tab 焦点行为、无横向溢出、可访问性快照和清洁页面日志检查；证据独立记录于 `ui-accessibility-evidence-v78.md`。

## v79 Dashboard 滚动态导航安全层

- UI-1：保持 AI TOKEN 品牌区在静止与滚动态的透明透景；仅为右侧 nav/account cluster 增加低 alpha、pointer-inert 的局部 reading rail，避免 CTA 与导航穿透叠层。
- UI-2：变更集中在 `responsive-tuning.css` 的滚动态伪元素与平板断点，不改变 DOM、导航契约、业务状态、API、数据流或认证；forced-colors 显式隐藏装饰 rail。
- UI-3：真实 5175 隔离实例已完成 320/390/768/1024/1440px 顶部矩阵、768/1440px 滚动态截图与 computed-style、无横向溢出、可访问性快照和清洁页面日志检查；证据独立记录于 `ui-accessibility-evidence-v79.md`。

## v80 空数据图表信号面

- UI-1：将零数据趋势/模型图表从空黑画布收敛为低对比的 waiting instrument；网格、零基线、扫描线和导引环服务于状态识别，不与空态文案抢层级。
- UI-2：变更集中在 `responsive-tuning.css`，复用现有 `.chart-wrap[data-chart-state="empty"]` 状态边界；不改变 DOM、Chart.js、API、导航、认证或数据流，forced-colors 和 reduced-motion 均有明确复位。
- UI-3：真实 5176 隔离实例已完成 320/390/768/1024/1440px 矩阵、1440px 滚动态截图、Week/Today 周期交互、可访问性快照和清洁页面日志检查；证据独立记录于 `ui-accessibility-evidence-v80.md`。

## v81 滚动态品牌阅读边界

- UI-1：保持整条 AI TOKEN 顶栏透明，只在滚动态品牌锁定区提供低 alpha 透景 lens，并移除已经完成任务的 `Scroll to explore` cue，避免 section copy 穿过身份标识。
- UI-2：变更集中在 `responsive-tuning.css` 的 `.site-header.is-scrolled .brand` 与相邻 `.scroll-cue` 表现层；不改变 DOM、布局盒、导航、API、认证或数据流，forced-colors 显式隐藏 lens。
- UI-3：真实 5177 隔离实例已完成 320/390/768/1024/1440px 顶部矩阵、390/1440px 滚动态截图、Week/Today 周期交互、可访问性快照和清洁页面日志检查；证据独立记录于 `ui-accessibility-evidence-v81.md`。

## v82 指针跟随生命周期

- UI-1：将停留在最后坐标的 pointer aura 收敛为移动时出现、静止后隐藏的轻量反馈，避免亮斑遮挡图表、表单和历史内容。
- UI-2：变更集中在 `motion.js` 的 pointer lifecycle 与 `style.css`/`scene-motion.css` 的 visibility 层；复用现有 pointer 节点和 reduced-motion/fine-pointer 门禁，不新增依赖、DOM、API、认证或数据流。
- UI-3：真实 5178 隔离实例已确认 default hidden → active visible → idle hidden → re-activate visible、无横向溢出、Week/Today 周期交互、可访问性快照和清洁页面日志；本轮四档 viewport 与真实辅助偏好仍按独立门禁记录，证据见 `ui-accessibility-evidence-v82.md`。

## v83 AI TOKEN 顶栏完全透景

- UI-1：移除滚动态右侧 reading rail 与品牌 lockup lens，解决顶部 `AI TOKEN` 行被误读为半透明深色卡片的问题；保留文字 keyline、滚动进度线和系统配色降级。
- UI-2：变更集中在 `responsive-tuning.css` 的伪元素表现层；不改变 DOM、布局盒、导航、认证、API、数据流或业务状态，所有文件仍低于 1000 行。
- UI-3：真实 5179 隔离实例已确认默认桌面首屏与 `scrollY = 720` 的透明 computed style、截图、Week/Today、无横向溢出、可访问性树和清洁页面日志；viewport override、真实设备、辅助偏好、Provider 联调和正式部署仍不宣称通过，证据见 `ui-accessibility-evidence-v83.md`。

## v84 TOTAL SIGNAL 内部扫描弧

- UI-1：在现有 TOTAL SIGNAL 核心内部加入一条低 alpha、transform-only 的扫描弧，让零数据状态也拥有连续但克制的仪表反馈，不增加视觉噪声或文字覆盖。
- UI-2：变更集中在 `scene-motion.css` 的装饰伪元素、forced-colors 与 reduced-motion 表现边界；不改变 DOM、API、业务状态、认证、数据流或依赖。
- UI-3：真实 5181 隔离实例已确认首屏截图、`core-sweep` computed animation 与时间采样、无横向溢出、可访问性语义和清洁页面日志；周期 pill 的异步同步观察已独立记录，证据见 `ui-accessibility-evidence-v84.md`。

## v85 周期 pill 同步

- UI-1：修复周期文字已切换而 active pill 仍停留在上一个周期的视觉不同步；同步测量让状态反馈在同一任务内落位，保留下一帧响应式校准。
- UI-2：变更集中在 `range-switcher.js` 的 active 状态编排，不改变摘要请求、period 契约、DOM、ARIA、认证或数据流；没有新增依赖。
- UI-3：真实 5181 隔离实例已确认四档 period 的 active/几何/状态文案、ARIA pressed、无横向溢出、顶栏透明回归和页面日志；证据见 `ui-accessibility-evidence-v85.md`，真实设备与辅助偏好继续独立门禁。

## v86 滚动态透明顶栏阅读层

- UI-1：将深度滚动时穿过 AI TOKEN sticky 顶栏的内容收敛为无填充背景模糊，保持品牌行本身完全透明，避免黑色横条和标题/CTA 争夺阅读焦点。
- UI-2：变更集中在后加载的 `responsive-tuning.css` 滚动态规则；`scene-motion.css` 继续提供基础透明默认值，不重复承担覆盖职责。不改变 DOM、导航、认证、API、业务状态或数据流；forced-colors 继续回到系统 Canvas。
- UI-3：真实 5182 隔离实例已确认首屏、720px、900px、1118px 滚动态截图与 computed style、标题安全落点、cue 退出、可访问性语义和清洁页面日志；证据见 `ui-accessibility-evidence-v86.md`，viewport override、真实设备与辅助偏好继续独立门禁。

## v87 品牌区透景与 CTA 收束

- UI-1：移除整条滚动态 header blur 对品牌区造成的亮色扩散；AI TOKEN 完整透景，右侧导航/账户保留局部透明镜片，hero CTA 在离开首屏后有节奏地退出阅读区。
- UI-2：变更集中在 `responsive-tuning.css` 的滚动态伪元素、CTA 状态和 reduced-motion/forced-colors 表现边界；`:focus-within` 不隐藏正在操作的 CTA，不改变 DOM、导航、认证、API、业务状态或数据流。
- UI-3：真实 5183 隔离实例已确认首屏、720px、2220px、返回顶部恢复、透明 computed style、局部镜片、CTA/cue 状态、焦点控件标签、无横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v87.md`，深度内容自动避让、viewport override、真实设备与辅助偏好继续独立门禁。

## v88 sticky 内容避让

- UI-1：新增独立 `sticky-occlusion.js` 表现模块，解决深度滚动时下方提交按钮穿过透明 AI TOKEN sticky 行的问题；非焦点控件收束视觉，焦点控件保持可见和可操作。
- UI-2：模块只依赖 header 几何、滚动状态和现有静态候选集合，通过表现类与 CSS 边界工作；不改变 DOM、API、认证、业务状态或数据流，hero actions 继续由 v87 专属规则处理。
- UI-3：真实 5184 隔离实例已确认首屏/720px/2220px、`#proxy-submit` 避让、Tab 焦点恢复、透明 computed style、无横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v88.md`，viewport override、真实设备与辅助偏好继续独立门禁。

## v89 预落位 reveal 可读性

- UI-1：提高 below-fold reveal 的预落位可读地板，避免内容已经进入视口却仍以低透明度显示；保留短距离、短时长的层次动画。
- UI-2：变更集中在 `responsive-tuning.css` 的 `prefers-reduced-motion: no-preference` 表现规则，不改变 DOM、IntersectionObserver、导航、API、认证、业务状态或数据流；移动端和 forced-colors 边界保持原契约。
- UI-3：真实 5185 隔离实例已确认 `scrollY=720/1200` 的预落位与 settled computed style、透明顶栏、42 个可见焦点控件、无横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v89.md`，viewport override、真实设备与辅助偏好继续独立门禁。

## v90 周期切换器透景层

- UI-1：将周期切换器外壳从偏黑高 alpha 砖块收敛为低 alpha 深蓝透景层，active lime pill 继续作为唯一主状态锚点。
- UI-2：变更集中在 `responsive-tuning.css` 的 `forced-colors: none` 表现规则，不改变 period API、状态编排、DOM、认证、业务数据或请求；系统配色不被新规则覆盖。
- UI-3：真实 5186 隔离实例已确认首屏/滚动态截图、四档 period 点击、ARIA pressed、42 个可见焦点控件、无横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v90.md`，viewport override、真实设备与辅助偏好继续独立门禁。

## v91 sticky 标题阅读边界

- UI-1：解决深度滚动时分析卡片标题穿过透明 sticky 顶栏并被截断的问题，复用现有避让策略，不引入新的视觉遮罩。
- UI-2：变更集中在 `sticky-occlusion.js` 候选集合与 `responsive-tuning.css` 表现契约；保留 DOM、辅助语义、焦点路径、forced-colors、reduced-motion、API、认证和业务数据边界。
- UI-3：真实 5187 隔离实例已确认 `scrollY=1118` 避让、`scrollY=898` 恢复、42 个可见焦点控件、1 个 h1/8 个 h2、无横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v91.md`，viewport override、真实设备与辅助偏好继续独立门禁。

## v92 自动采集操作行阅读边界

- UI-1：解决深度滚动时提交按钮已隐藏但隐私说明仍穿过透明顶栏的残余视觉问题，将操作行作为完整层级处理。
- UI-2：变更集中在 `sticky-occlusion.js` 候选集合与既有 CSS 表现类；不改变 DOM、Tab 顺序、表单值、焦点语义、forced-colors、reduced-motion、API、认证或业务数据。
- UI-3：真实 5188 隔离实例已确认 `scrollY=2200` 避让、提交按钮 Tab 焦点恢复、离开交叠带恢复、42 个可见焦点控件、无横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v92.md`，更上方字段残影和其他外部门禁继续独立记录。

## v93 透明品牌行表面边界

- UI-1：解决透明 AI TOKEN 行下方深蓝卡片穿过 sticky 顶栏造成的“顶栏不透明”观感；表面仅在交叠带局部裁切，背景插画恢复连续透景。
- UI-2：变更集中在 `sticky-occlusion.js` 的表面交叠测量与 `responsive-tuning.css` 的 mask 表现；`:focus-within` 清除裁切，保留字段焦点、DOM、Tab 顺序、API、认证、业务数据、reduced-motion 和 forced-colors 边界。
- UI-3：真实 5189 隔离实例已确认 `scrollY=2200` 顶栏透明、76px 局部裁切、备注字段焦点恢复、离开交叠带恢复、40 个可见焦点控件、无横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v93.md`，viewport override、真实设备与辅助偏好继续独立门禁。

## v94 深滚孤立表面边界

- UI-1：解决深滚时卡片只剩一小段位于透明 AI TOKEN 行下方、视觉上形成第二条黑色顶栏的问题；不扩大顶栏背景，而是让孤立表面作为一个完整单元退出。
- UI-2：变更集中在 `sticky-occlusion.js` 的 120px fragment 判断和 `responsive-tuning.css` 的焦点安全表现；完整交叠仍采用局部 mask/feather，不改变 DOM、Tab 顺序、API、认证、表单值、reduced-motion 或 forced-colors 边界。
- UI-3：真实 5190 隔离实例已确认 `scrollY=2200` 完整过渡后残片与动作行隐藏、Tab 到备注字段并恢复、中间滚动主体可见、40 个可见焦点控件、无横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v94.md`，viewport override、真实设备与辅助偏好继续独立门禁。

## v95 长页面 reveal 生命周期

- UI-1：解决快速跳过长页面区块后 `IntersectionObserver` 漏回调、返回时仍保留预落位灰度的问题，保证滚动路径上的内容最终完整可读。
- UI-2：变更集中在 `motion.js`；通过单帧 scroll/resize 同步器补齐已越过阈值的区块，清除 transition delay 并解除 observer，不改变 DOM、Tab 顺序、API、认证、表单值、reduced-motion 或 forced-colors 边界。
- UI-3：真实 5191 隔离实例已确认深度快跳约 `scrollY=2995` 与返回 `scrollY=1200` 后 8 个 reveal 区块均为 `opacity=1 / transform=none`，39 个交互控件具备名称、无横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v95.md`，viewport override、真实设备与辅助偏好继续独立门禁。

## v97 窄短手机首屏动作区

- UI-1：解决 `320×720` 窄短手机首屏两个主操作被 hero stage 推到视口外的问题，保留背景场景、TOTAL SIGNAL 轨道、标题和阅读顺序。
- UI-2：变更集中在 `responsive-tuning.css` 的窄短手机 media query；只收紧 stage spacing、orbit width、lede margin 和 footer gap，不改变 DOM、Tab 顺序、API、认证、表单值、桌面/平板/普通手机布局或 reduced-motion/forced-colors 边界。
- UI-3：真实 5193 隔离实例已确认 `320/360/390/768/1024/1440` 六档无横向溢出，320px 主操作 `bottom=698` 位于视口内，41 个交互控件具备名称和清洁页面日志；证据见 `ui-accessibility-evidence-v97.md`，真实设备与辅助偏好继续独立门禁。

## v98 指标栏局部透景层

- UI-1：解决移动端指标栏在人物、代码和设备插画上扫读困难的问题；保留整条透明场景与三段指标的原有信息优先级。
- UI-2：变更集中在 `responsive-tuning.css` 的 `.signal-cell` 表现规则；低 alpha 渐变提供局部阅读边界，确认会形成暗带的 blur 已移除，不改变 DOM、Tab 顺序、API、认证、表单值、桌面/移动布局或 reduced-motion/forced-colors 边界。
- UI-3：真实 5194 隔离实例已确认 `320/390/768/1024/1440` 五档无横向溢出、指标单元与顶栏均为无 blur、39/39 当前控件具备名称和清洁页面日志；证据见 `ui-accessibility-evidence-v98.md`，真实设备与辅助偏好继续独立门禁。

## v99 AI TOKEN 品牌行透明修正

- UI-1：解决最上方 AI TOKEN 行仍像黑色覆盖层的问题；品牌 header 保持完整透景，hero 元信息只保留局部短信号线，不再使用贯穿整屏的底边线。
- UI-2：变更集中在 `responsive-tuning.css` 的普通配色表现规则；不改变 DOM、布局盒、导航、周期 API、认证、表单值、数据流或 Tab 顺序，forced-colors 继续由系统配色接管。
- UI-3：真实 5195 隔离实例已确认 `320/390/768/1024/1440` 五档透明 computed style、390px 滚动态、Today/Week/Month/All time 四档状态、无横向溢出、36/36 可见控件命名和清洁页面日志；证据见 `ui-accessibility-evidence-v99.md`，真实设备与辅助偏好继续独立门禁。

## v100 顶部状态胶囊透明修正

- UI-1：解决 hero 元信息右侧“已更新：今天”仍像深色玻璃卡片的问题；状态点、边界与错误语义保留，背景场景连续透过顶部信息层。
- UI-2：变更集中在 `responsive-tuning.css` 的普通配色表现规则，并补强 `OBSERVATORY` 副标题 keyline；不改变 DOM、布局盒、导航、周期 API、认证、表单值、数据流或 Tab 顺序，forced-colors 继续由系统配色接管。
- UI-3：真实 5200 隔离实例已确认 `320/390/768/1024/1440` 五档透明 computed style、`scrollY=720` 滚动态、Today/Week/Month/All time 四档状态、无横向溢出、40/40 可见控件命名和清洁页面日志；证据见 `ui-accessibility-evidence-v100.md`，真实设备与辅助偏好继续独立门禁。

## v101 登录认证卡片透景材质

- UI-1：解决登录页认证卡片仍像黑色大块的问题；使用低 alpha 深蓝透景层让角色、代码和设备场景参与层次，同时保留字段的稳定阅读面。
- UI-2：变更集中在 `responsive-tuning.css` 的普通配色 `.auth-card` 与 `:focus-within` 表现规则；不改变 DOM、认证 API、字段、焦点顺序、导航、数据流或 forced-colors/reduced-motion 边界。
- UI-3：真实 5201 隔离实例已确认 `320/390/768/1024/1440` 五档卡片几何、`blur(14px)`、无横向溢出、聚焦状态、7/7 可见控件命名和清洁页面日志；证据见 `ui-accessibility-evidence-v101.md`，真实设备与辅助偏好继续独立门禁。

## v102 周期切换器局部透景

- UI-1：解决 Dashboard 周期切换器仍像深色模糊浮层、与透明 AI TOKEN 品牌行材质断层的问题；保持 active lime 状态锚点与四档周期语义。
- UI-2：变更集中在 `responsive-tuning.css` 的普通配色 `.range-switcher` 表现规则，将外壳改为低 alpha 垂直渐变并移除 blur；不改变 DOM、Tab 顺序、API、认证、表单值、数据流或 forced-colors/reduced-motion 边界。
- UI-3：真实 5202 隔离实例已确认 `320/390/768/1024/1440` 五档无横向溢出、Today/Week/Month/All time 四档真实点击与 `aria-pressed` 同步、36/36 或 40/40 当前控件命名、1 个 h1/8 个 h2、桌面与深滚截图和清洁页面日志；证据见 `ui-accessibility-evidence-v102.md`，真实设备与辅助偏好继续独立门禁。

## v103 认证卡片局部透景

- UI-1：解决登录/注册认证卡片仍形成偏重深色矩形、遮断角色和设备场景的问题；保留输入字段的稳定阅读面与认证操作优先级。
- UI-2：变更集中在 `responsive-tuning.css` 的普通配色 `.auth-card` 与 `:focus-within` 表现规则，将表面收敛为 `.54/.66` alpha、`blur(11px) saturate(1.06)`；不改变 DOM、认证 API、字段、Tab 顺序、数据流或 forced-colors/reduced-motion 边界。
- UI-3：真实 5203 隔离实例已确认 `/login` 与 `/register` 在 `320/390/768/1024/1440` 五档无横向溢出、卡片 computed style、7/7 控件命名、1 个 h1/1 个 h2、登录字段焦点、桌面/移动截图和清洁页面日志；证据见 `ui-accessibility-evidence-v103.md`，真实设备与辅助偏好继续独立门禁。

## v104 Dashboard 内容卡片透景

- UI-1：解决 Dashboard 进入图表和自动采集区后连续深色玻璃表面压过插画、滚动层级突然变重的问题；保留内容卡片作为稳定阅读单元。
- UI-2：变更集中在 `responsive-tuning.css` 的普通配色图表/表单/说明/记录/明细卡片表现规则，将外壳收敛为 `.70/.82` alpha、`blur(12px) saturate(1.06)`；不改变 DOM、Tab 顺序、API、认证、表单值、数据流或 forced-colors/reduced-motion 边界。
- UI-3：真实 5204 隔离实例已确认 `320/390/768/1024/1440` 五档无横向溢出、卡片 computed style、Today/Week/Month/All time 四档真实点击、Base URL 字段焦点、36/37 或 40/41 当前控件命名、1 个 h1/8 个 h2、桌面/移动深滚截图和清洁页面日志；证据见 `ui-accessibility-evidence-v104.md`，真实设备与辅助偏好继续独立门禁。

## v105 自动采集连接状态透景

- UI-1：解决自动采集标题旁 `未连接` 状态仍像旧深色模糊胶囊、与顶部透明材质体系断层的问题；保留状态扫描优先级和 ready/error 语义。
- UI-2：变更集中在 `responsive-tuning.css` 的普通配色 `.connection-badge`、`.is-ready` 和 `.is-error` 表现规则，统一为低 alpha、无 blur 的局部透景层；不改变 DOM、Tab 顺序、API、认证、表单值、数据流或 forced-colors/reduced-motion 边界。
- UI-3：真实 5205 隔离实例已确认 `320/390/768/1024/1440` 五档无横向溢出、状态 computed style、Dashboard 卡片 `blur(12px)`、Base URL 字段焦点、36/37 或 40/41 当前控件命名、1 个 h1/8 个 h2、深滚截图和清洁页面日志；证据见 `ui-accessibility-evidence-v105.md`，真实设备与辅助偏好继续独立门禁。

## v106 顶部元信息边界清理

- UI-1：将 `.hero-topline` 的继承边线归零，只保留局部 signal trace，使透明品牌行、角色场景和元信息保持同一透景材质。
- UI-2：本轮仅调整 `responsive-tuning.css` 的边界参数，不新增监听器、计时器、依赖或动画；既有 pointer/reveal、键盘焦点、reduced-motion 和 forced-colors 契约保持。
- UI-3：5206 隔离浏览器确认 `320/390/768/1024/1440` 五档四边透明、无横向溢出、周期交互、深滚层级和清洁页面日志；真实设备与辅助偏好仍待独立门禁。

## v107 首屏说明文案可读性

- UI-1：为跨越角色与笔记本高光的 `.hero-lede` 增加局部两级文字 keyline，不引入额外面板，保持透明场景与 Moonshot-inspired 首屏构图。
- UI-2：本轮仅调整 `responsive-tuning.css` 的文本表现参数，不新增监听器、计时器、依赖或动画；既有 pointer/reveal、键盘焦点、reduced-motion 和 forced-colors 契约保持。
- UI-3：5208 隔离浏览器确认 `320×720/390×844/768/1024/1440` 五档 keyline、无横向溢出、周期交互、深滚层级和清洁页面日志；真实设备与辅助偏好仍待独立门禁。

## v108 认证辅助文案层级

- UI-1：登录/注册共用认证卡片的 helper copy 与 alternate-route link 提升到可读语义色，使用局部字形 keyline 和细 lime 下划线，保持透明玻璃与角色场景连续。
- UI-2：本轮仅调整 `responsive-tuning.css` 的普通配色表现，不新增监听器、计时器、依赖或业务分支；既有入场、轨道、焦点和 reduced-motion 契约保持。
- UI-3：5209 隔离浏览器确认登录/注册 `320/390/768/1440` 四档无横向溢出、链接可发现、用户名焦点状态和清洁页面日志；真实设备与辅助偏好仍待独立门禁。

## v109 周期控件 sticky 边界

- UI-1：周期切换器在真实穿过透明 AI TOKEN 顶栏时退出视觉层，避免与导航文字叠层；未交叠时保持原有透明仪表盘构图。
- UI-2：复用 `sticky-occlusion.js` 既有 requestAnimationFrame 调度和 `:focus-within` 恢复，不新增监听器、计时器、依赖或动画；reduced-motion/forced-colors 边界保持。
- UI-3：5210 隔离浏览器确认 `320/390/768/1024/1440` 五档无横向溢出、交叠边界、周期焦点恢复和清洁页面日志；真实设备与辅助偏好仍待独立门禁。

## v110 顶部品牌栏透光层

- UI：将 AI TOKEN 顶栏回归为低 alpha 透光渐变，移除模糊横带，保持品牌层与背景插画的连续性。
- 前端：仅修改共享 `responsive-tuning.css`，未改变模板、交互、接口或 sticky 避让逻辑。
- 后端：无后端改动；隔离数据库和受保护服务保持不变。
- 架构：沿用“共享视觉契约 + forced-colors 隔离 + 独立证据”的边界，文件行数保持在项目硬门禁内。
## v111 认证卡片 signal line 呼吸

- UI：只增强认证卡片已有顶部 signal line 的低频呼吸，不新增视觉组件，不改变版式。
- 前端：动效限定在 `@media (forced-colors: none)` 与 `prefers-reduced-motion: no-preference`，`:focus-within` 自动恢复原有 focus 视觉。
- 后端：无后端、接口、数据库改动。
- 架构：共享样式仍位于 `responsive-tuning.css`，保留低耦合、文件行数和回滚边界。
## v112 Dashboard 指标栏 sticky 避让

- UI：深滚时指标单元不再与透明 AI TOKEN 顶栏文字叠层，实际进入 `76px` 阅读带后整体退出，离开后保持原有透明构图。
- 前端：将 `.signal-cell` 加入 `sticky-occlusion.js` 既有候选集合，沿用 requestAnimationFrame 调度、统一淡出规则和 `focus-within` 恢复；无新增监听器、依赖或页面脚本。
- 后端：无后端、接口、数据库改动。
- 架构：变更只涉及共享遮挡选择器与共享过渡样式，遵守低耦合、文件行数和回滚边界；真实隔离 5213 证据见 `docs/ui-accessibility-evidence-v112.md`。
## v113 AI TOKEN 顶栏真正透明

- UI：移除普通配色下顶栏的低 alpha 渐变和内侧阴影，背景插画可完整穿过 AI TOKEN 行；品牌文字 keyline 与滚动进度线保留。
- 前端：只修改共享 `responsive-tuning.css` 的顶栏视觉契约，不改变模板、导航、滚动、认证和 API；forced-colors 分支继续由系统颜色接管。
- 后端：无后端、接口、数据库改动。
- 架构：单文件局部变更，依赖方向、文件行数和回滚边界保持；5214 浏览器证据见 `docs/ui-accessibility-evidence-v113.md`，滚动进度线 `0px` 异常单独进入下一轮。
## v114 异步高度下的滚动进度同步

- UI：进度线在异步内容加载、记录刷新和异常补录展开后保持真实比例，使用固定轨道与 `scaleX`，不增加新的装饰层。
- 前端：`navigation.js` 复用现有 `requestAnimationFrame` 调度并按能力启用 `ResizeObserver`；`ui-polish.css` 只将进度线从 `width` 过渡改为 `transform`，reduced-motion 继续关闭过渡。
- 后端：无后端、接口、数据库改动。
- 架构：观察页面尺寸而非业务状态，保持 UI→导航模块边界，无轮询、无循环依赖、文件行数和回滚边界符合门禁；5215 证据见 `docs/ui-accessibility-evidence-v114.md`。
## v115 透明品牌行光学边界

- UI：透明 AI TOKEN 行增加单条 1px hairline 和品牌标记的 hover/focus 光学反馈；本体依旧无背景填充、无 blur、无 shadow，深滚不形成第二条黑色横带。
- 前端：新增 `header-chrome.css` 并在 `base.html` 末尾接入，使用伪元素、`opacity`、`transform` 和既有 focus contract；无新增业务监听器、API 或 DOM 节点。
- 后端：无后端、接口、数据库改动。
- 架构：品牌 chrome 与通用表面样式解耦，模块边界、forced-colors/reduced-motion 降级、文件行数和回滚路径清晰；5216 证据见 `docs/ui-accessibility-evidence-v115.md`。
## v116 Dashboard 空态观测舱

- UI：无数据图表使用低 alpha 观测面、细网格、`NO SIGNAL / READY` 和单一 signal line，首屏插画与分析区材质连续；不伪造数据。
- 前端：新增 `observatory-signal.css`，仅消费既有 `.is-empty/.is-unavailable`、`data-chart-state` 和 `.range-switcher` DOM contract；不新增脚本监听器或 API。
- 后端：无后端、接口、数据库改动。
- 架构：职责限定于 `03-observatory`，与 Chart.js renderer 和业务状态解耦；forced-colors/reduced-motion、文件行数和回滚路径清晰；5217 证据见 `docs/ui-accessibility-evidence-v116.md`。

## v117 AI TOKEN 品牌栏最终透明契约

- UI-1：解决最上方 AI TOKEN 行在多层视觉规则叠加后产生黑色覆盖错觉的问题；品牌栏继续作为背景插画的透明窗口，保留轻量边界和字形可读性。
- UI-2：新增 `brand-transparency.css`，通过最后加载的共享 CSS contract 统一 header、brand、导航与账户子级的透明、无 blur、无 shadow 规则；forced-colors 与 reduced-motion 单独降级。
- UI-3：真实隔离 5218 已确认 `320/390/768/1024/1440` 五档无横向溢出、首屏/滚动态/窄屏截图与透明 computed style，应用日志无 error/warn；证据见 `ui-accessibility-evidence-v117.md`。
- 后端：无后端、数据库、接口、认证或密钥处理改动；保护端口 `5000/5011` 未触碰。
- 架构师：模块职责限定在 `01-shell`，不向业务脚本引入依赖；新增文件低于 1000 行，回滚边界为 stylesheet link 与独立模块。

## v118 认证字段 signal lane

- UI-1：解决登录/注册卡片内字段连续、层级弱和焦点反馈不明显的问题；新增流程编号与局部 signal line，保持场景透景和触控密度。
- UI-2：新增 `auth-signal.css`，登录/注册模板仅增加 `auth-field-label` 视觉结构；编号使用 `aria-hidden`，字段仍由原生 label 提供唯一可访问名称。
- UI-3：真实隔离 5219 已确认 `320/390/768/1024/1440` 五档无横向溢出、登录/注册继承、焦点 signal、标题层级和应用日志；证据见 `ui-accessibility-evidence-v118.md`。
- 后端：无后端、数据库、接口、认证、CSRF 或密钥处理改动；保护端口 `5000/5011` 未触碰。
- 架构师：模块职责限定在 `02-auth`，不向业务脚本引入依赖；新增 CSS/证据文件低于 1000 行，回滚边界为样式链接、模板小结构和独立模块。

## v119 自动采集协议 sticky rail

- UI-1：解决自动采集长表单与 `HOW IT WORKS` 说明在深滚时失去上下文的问题；桌面协议栏保持三步流程可见，移动端不增加额外固定层。
- UI-2：新增 `connect-rail.css`，只通过 `.tool-grid.auto-layout > .guide-card` 的桌面媒体查询建立 sticky、signal rail 和低 alpha 阅读面；901px 以下、forced-colors、reduced-motion 均显式降级。
- UI-3：真实隔离 5220 已确认 `320/390/768/1024/1440` 五档无横向溢出、1440px sticky 深滚、Base URL 焦点恢复、390px 长表单和应用日志；证据见 `ui-accessibility-evidence-v119.md`。
- 后端：无后端、数据库、接口、认证、Key 生命周期或数据流改动；保护端口 `5000/5011` 未触碰。
- 架构师：模块职责限定在 `04-connect`，不向业务脚本引入依赖；新增 CSS/证据文件低于 1000 行，回滚边界为 stylesheet link 与独立模块。

## v120 顶部品牌场景兼容边界

- UI-1：解决透明 AI TOKEN 品牌行下方因旧个人入口缺少场景尺寸而显示黑色画布的问题；品牌行继续作为场景窗口，截图确认代码屏、角色和设备可见。
- UI-2：基础 `style.css` 只补 `.story-backdrop` / `.story-backdrop-image` 的定位、尺寸、背景图和层级；当前完整 scene module 继续拥有动画、偏移和遮罩，不复制交互逻辑。
- UI-3：真实 `5000` 与隔离 `5221` 已确认 `320/390/768/1024/1440` 五档、滚动态、透明 computed style、背景图加载、横向溢出和清洁页面日志；证据见 `ui-accessibility-evidence-v120.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Key 生命周期或数据流改动；保护端口 `5000/5011` 未触碰。
- 架构师：修复归属 `01-shell` 基础展示边界，复用现有场景 DOM，不新增依赖；基础 CSS 总计 480 行，回滚边界为兼容样式块。

## v121 移动认证入口节奏

- UI-1：解决移动端登录/注册卡片开始位置过低、主要输入任务需要过长滚动的问题；保留完整品牌故事、信任信息和场景层。
- UI-2：只在基础 `style.css` 的 `620px` / 常规手机高度媒体查询中调整 gap、padding、标题/说明/信任线节奏；不改变 DOM 和认证交互。
- UI-3：真实 `5000` 已确认登录/注册五档响应式、390px 卡片提前、768/1440 构图、键盘焦点、heading 结构、横向溢出和清洁日志；证据见 `ui-accessibility-evidence-v121.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Key 生命周期或数据流改动；保护端口 `5000/5011` 未触碰。
- 架构师：变化限定在 `02-auth` 响应式展示边界，无新依赖、无跨层耦合；`style.css` 共 550 行，回滚边界为 v121 媒体查询。
## v122 Dashboard 移动首屏节奏

- UI-1：解决 Dashboard 移动端 orbit 占据过多纵向空间、主要入口和周期控制被推到首屏之外的问题；建立“轨道—任务—控制”的连续阅读顺序。
- UI-2：在基础 `style.css` 与 `responsive-tuning.css` 的 `620px` 以下媒体查询中统一调整 orbit 上限、hero stage 最小高度、说明间距和底部节奏；不改变 DOM、周期脚本、接口或数据状态。
- UI-3：真实 `5000` 已确认六档响应式、390px 首屏几何、Week 交互、Dashboard heading 结构、深滚空态、无横向溢出和清洁日志；证据见 `ui-accessibility-evidence-v122.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Key 生命周期或数据流改动；保护端口 `5000/5011` 未触碰。
- 架构师：变化限定在 `03-observatory` 响应式展示边界；两层 CSS 保持同一契约，无新依赖、无跨层耦合，`style.css` 587 行、`responsive-tuning.css` 691 行，回滚边界为 v122 两个媒体查询块。

## v123 Sticky 顶栏兼容遮挡边界

- UI-1：解决旧个人启动器下透明 AI TOKEN 顶栏与深滚分析标题重叠、标题穿透阅读带的问题；不增加黑色背景，保留场景透视。
- UI-2：基础 `style.css` 复用既有 sticky occlusion class、自定义变量和 focus contract，补齐 mask、淡出、reduced-motion 与 forced-colors 边界；不改 DOM、脚本、接口或数据。
- UI-3：真实 `5000` 已确认深滚分析区、透明顶栏、六档响应式、Week/键盘焦点、heading 结构和清洁日志；证据见 `ui-accessibility-evidence-v123.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Key 生命周期或数据流改动；保护端口 `5000/5011` 未触碰。
- 架构师：变化限定在 `01-shell` sticky chrome compatibility boundary；无新依赖、无跨层耦合，`style.css` 673 行，回滚边界为 v123 基础样式块。

## v124 分享链接章节定位

- UI-1：解决分享链接因异步内容高度变化而落在错误章节、标题被顶栏遮挡或出现回弹的问题；首次进入即可看到对应上下文。
- UI-2：`navigation.js` 新增有时限 `settleInitialHash`，复用目标 `scrollMarginTop` 和 body `ResizeObserver`；初始化 hash 使用即时滚动，连续稳定或用户交互后恢复平滑滚动。
- UI-3：全新浏览器上下文已确认 `#connect/#activity/#history`、无 hash 首屏、Connect 点击导航、滚动策略释放、heading 结构、无横向溢出和清洁日志；证据见 `ui-accessibility-evidence-v124.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Key 生命周期或数据流改动；保护端口 `5000/5011` 未触碰。
- 架构师：变化限定在 `01-shell / navigation contract`，无新依赖、无跨层耦合，当前 `navigation.js` 260 行、`style.css` 628 行，回滚边界为 v124 导航函数与滚动 class。

## v125 AI TOKEN 顶部半透明玻璃层

- UI-1：修复最上方 AI TOKEN 行虽然“透明”但在深色场景中看起来像黑色覆盖带的问题；背景角色和设备轮廓继续可见，文字获得有限阅读底。
- UI-2：最终 `brand-transparency.css` 将强制透明规则改为低 alpha 深蓝渐变、`blur(11px)`、弱边界阴影与窄屏轻增强；`ui-polish.css` 同步提供旧启动器兼容层，移动媒体查询置于 `forced-colors` 之前，系统高对比度可安全覆盖。
- UI-3：5000/5011 返回新样式资源，Git diff/compileall/文本行数门禁通过；浏览器此前已确认品牌行几何不变，未新增 DOM 或交互状态，独立证据见 `ui-accessibility-evidence-v125.md`。
- 后端：无后端、数据库、接口、认证、CSRF、Provider 或 Key 生命周期改动；保护端口 `5000/5011` 未触碰。
- 架构师：变化限定在 `01-shell` 视觉契约，保留独立回滚点；`brand-transparency.css` 91 行，未引入跨层依赖，forced-colors/reduced-motion 边界明确。

## 集成闸门

- 共享运行代码仍集中在当前 checkout，没有创建 worktree 或角色复制源代码。
- Provider adapter 当前实现 `auto → openai-compatible`；新增厂商应新增 adapter，不应在 Flask 路由增加分支。
- 交付层通过 `release-doctor.ps1/.bat` 编排单项门禁；它只报告源码、工具链和部署状态，不安装软件或改变主机状态（ADR-071）。
- 自动采集不猜测缺失的 usage；没有 usage 的 provider 仍提示异常补录。
- 当前未用真实 provider Key 做上游调用验证，因此“真实 usage 入库”依赖用户提供合法 Base URL/Key，不能用演示数据冒充通过。
- 外部客户端的 per-user Usage Ingest Token、固定 `/api/v1/ingest/usage` 边界和本地 Gateway 已实现；Kimi Code 等 stock 客户端的真实 provider 联调仍需授权 Key，provider Key 隔离遵守 ADR-054/056/058/059/075。
