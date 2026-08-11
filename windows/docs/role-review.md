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

## 集成闸门

- 共享运行代码仍集中在当前 checkout，没有创建 worktree 或角色复制源代码。
- Provider adapter 当前实现 `auto → openai-compatible`；新增厂商应新增 adapter，不应在 Flask 路由增加分支。
- 交付层通过 `release-doctor.ps1/.bat` 编排单项门禁；它只报告源码、工具链和部署状态，不安装软件或改变主机状态（ADR-071）。
- 自动采集不猜测缺失的 usage；没有 usage 的 provider 仍提示异常补录。
- 当前未用真实 provider Key 做上游调用验证，因此“真实 usage 入库”依赖用户提供合法 Base URL/Key，不能用演示数据冒充通过。
- 外部客户端的 per-user Usage Ingest Token、固定 `/api/v1/ingest/usage` 边界和本地 Gateway 已实现；Kimi Code 等 stock 客户端的真实 provider 联调仍需授权 Key，provider Key 隔离遵守 ADR-054/056/058/059/075。
