# Web UI-3 响应式与可访问性证据

**作者：** AI Token Tracker Engineering Team  
**日期：** 2026-08-10  
**范围：** Windows 登录页、场景背景层、认证表单、窄屏布局

## 执行方式

使用本机 5011 端服务和 Chrome 151 的 DevTools Protocol 设备指标覆盖执行运行时检查。当前环境未提供 Chrome DevTools MCP，因此使用等价的本地 CDP 连接；只访问 `127.0.0.1`，不读取 Cookie、localStorage、密码或令牌。

## 320px 运行时结果

| 检查项 | 结果 |
| --- | --- |
| CSS viewport | `innerWidth=320`，`innerHeight=900` |
| 可视布局宽度 | `clientWidth=305`；差值来自垂直滚动条 |
| 横向溢出 | `body.scrollWidth=305`、`documentElement.scrollWidth=305`，通过 |
| 认证布局 | shell `275px`，卡片 `275px`，均在可视布局范围内 |
| 标签关联 | `用户名 → username`、`密码 → password`，通过 |
| 焦点序列 | `username → password → submit → Create one → 隐私与数据说明`，通过 |
| reduced motion | `prefers-reduced-motion: reduce` 命中；背景和轨道动画计算值为 `none` |
| 浏览器运行时 | 登录页无 console exception、无 failed request、无 4xx/5xx response |

此前 320px 检查发现背景 fixed 层与认证页装饰 pseudo-layer 把文档扩大到 340px。本切片通过窄屏背景边界收敛和认证装饰层 `overflow: clip` 修复了具体来源，没有使用全局遮罩掩盖业务内容溢出。

## 语义与对比度结果

- 页面存在一个 `h1` 和一个认证 `h2`；背景、轨道和鼠标层均标记 `aria-hidden="true"`。
- 用户名、密码均使用可见 `label` 包裹控件；提交按钮为原生 `button`。
- 运行时颜色 token 的保守语义组合对比度：标题/画布 `18.76:1`、正文/画布 `14.37:1`、label/panel `13.19:1`、卡片标题/panel `17.22:1`、主按钮文字/按钮 `16.31:1`、placeholder/input `9.64:1`。
- 这些数值基于运行时 computed color 和语义面板色；背景插画仍由遮罩层和表面层隔离，不能把图片中的局部像素当作正文承载面。

## 截图证据

- [`ui-audit-v4-320.png`](../.cache/ui-audit-v4-320.png)：CDP 设备指标覆盖后的窄屏登录页。
- [`ui-audit-v4-768.png`](../.cache/ui-audit-v4-768.png)：移动/平板断点登录页。
- [`ui-audit-v4-1024.png`](../.cache/ui-audit-v4-1024.png)：平板/桌面过渡断点登录页。
- [`ui-audit-v4-1440.png`](../.cache/ui-audit-v4-1440.png)：桌面登录页。

## v6 背景与四档回归（历史记录）

- v6 资源已完成本地视觉检查：保持左侧登录负空间、右侧单角色构图，并强化笔记本、桌面工作站、Rust/RL telemetry 屏幕与服务器机架的层次。
- 历史 v6 CSS 曾切换到 `/static/assets/embedded-rust-engineer-bg-v6.png`；当前默认引用已由 v9 小节记录，v6 资产仍保留作为历史回滚版本。
- v6 仅有图像生成器和本地文件检查证据；当前环境没有可用浏览器 CDP，因此没有把旧 v5 截图冒充 v6 运行时证据。
- 下一次具备浏览器 CDP 运行时后，需要重新生成 `ui-audit-v6-1440/1024/768/320.png`，并复核四档横向溢出、空态、焦点和局部对比度。
- 本轮静态语义切片已通过：仪表盘只保留一个 `h1`、记录表列头和管理员详情表使用 `scope="col"`、表格有 caption，管理员详情打开后焦点进入关闭按钮并在关闭后返回触发按钮；登录/注册密码 maxlength 与服务端 256 字符边界一致。
- v4 截图保留为历史基线，不再作为 v6 默认渲染证据。

## v6.1 场景层增强（历史记录）

- 登录/注册页的 v6 角色场景增加 `RUST / RL WORKBENCH` 与
  `MACBOOK / STUDIO SCENE` 的低频场景签名；它标记为 `aria-hidden`，不伪装成真实
  硬件连接状态，也不参与业务状态判断。
- 认证卡片改为受控透明度并启用 `backdrop-filter`，让工作台插画可见，同时保留深色
  内容承载面、正文 token 和输入焦点边界；不支持滤镜的浏览器仍使用不透明度回退。
- 320px 额外收紧签名字距和字号，避免场景标签导致横向溢出；`prefers-reduced-motion`
  会关闭 beacon 脉冲。
- 使用隔离空 schema 的 Flask 渲染 smoke 检查 `/login`、`/register`、`/privacy` 均为
  `200`，v6 背景与场景签名均存在；没有创建真实用户或写入项目数据库。
- 透明卡片、pointer follower、背景视差和 reduced-motion 规则保持不变；下一次具备浏览器
  CDP 运行时后需要重拍 v6.1 四档截图并复核透明卡片后的局部对比度。

## v6.2 本机 Chrome 只读视觉回归（历史记录）

- 本轮使用本机 Chrome headless 对当前源码隔离服务 `http://127.0.0.1:5012/login` 生成了四档截图：
  [`login-v6-current-1440.png`](../.cache/ui-audit-v6/login-v6-current-1440.png)、
  [`login-v6-current-1024.png`](../.cache/ui-audit-v6/login-v6-current-1024.png)、
  [`login-v6-current-768.png`](../.cache/ui-audit-v6/login-v6-current-768.png) 和
  [`login-v6-current-320.png`](../.cache/ui-audit-v6/login-v6-current-320.png)。5011 为旧进程，
  不作为当前源码证据。
- 1440/1024/768 档确认 v6 场景、登录卡片、输入焦点和主要文本正常加载；本次没有提交
  真实账户、Key、Cookie 或 localStorage。
- 320px 文件只作为渲染器观察样本：当前 Chrome CLI 路径不是 DevTools Protocol 的
  设备指标覆盖，且自动聚焦会改变初始滚动位置，因此不把它当作横向溢出、焦点顺序或
  reduced-motion 的通过证据。移动端 CSS 已增加 `width/max-width/min-width` 收敛，真实
  320px 设备指标仍需可用 CDP 后复核。
- 本次静态回归通过：Python 模块编译、前端脚本语法、只读 `token_tracker audit --json`
  和 `git diff --check`；审计结果为 `pass=6 / pending=3 / fail=0`。

## v7 背景视觉升级（生成资产与静态证据）

- 历史阶段默认场景曾升级为 `embedded-rust-engineer-bg-v7.png`，Android 同步使用
  `embedded_rust_engineer_bg_v7.png`；两份 PNG 的 SHA-256 为
  `B48B415E8685B9CB8A32E000ABC148C124A5A7D171E441F03650CD6D30DA1926`。
- v7 右侧保留成年御姐气质的嵌入式 Rust/RL 工程师、银色 Pro 笔记本、桌面工作站、示波器和
  开发板；左侧维持低细节深色留白，便于正文和表单阅读。图片不含可读文案、logo 或真实连接状态。
- 已完成生成器输出与本地文件目视检查；本节不把 v6 截图冒充 v7 运行时截图。合法认证会话下的
  仪表盘、连接页、管理员页四档截图仍按 UI-3 门禁单独跟踪。

## v8 背景资产视觉升级（生成资产与静态证据）

- 历史阶段默认场景曾升级为 `embedded-rust-engineer-bg-v8.png`，Android 同步使用
  `embedded_rust_engineer_bg_v8.png`；两份 PNG 的 SHA-256 为
  `D85B08787BEC88B558E32BE99921F327EC099B3D2B550B0773143D2C7DF3BE56`。
- v8 提升了角色面部、银色 Pro 笔记本、桌面 Mac Studio 风格计算机、示波器与实验板的识别度，
  并保留左侧低细节深色留白；画面不包含可读文案、logo 或真实连接状态。
- 已完成生成器输出与 Web/Android 项目文件目视检查；v7 及之前版本继续作为回滚资产，
  合法认证会话下的受保护页面和真实设备视觉验收仍不因素材替换而自动通过。
- 当前源码使用隔离 SQLite 和临时端口 `5019` 观察到 `login-v8-source-1440.png`、
  `login-v8-source-768.png` 与 `login-v8-source-320.png`；1440 档确认主图已进入渲染层，
  768/320 档仅作为 headless renderer observation，不替代 CDP 或真实设备指标。
- 用户当前保持运行的旧 5000 端口进程仍返回 v7 改造前的模板（缺少显式 `<img>` 和场景样式），
  因此本轮不终止该进程；重启 Windows 服务后才会加载 v10 源码。5000/5011 均保持 loopback。

## v9 背景资产视觉升级（生成资产与静态证据）

- v9 阶段默认场景曾升级为 `embedded-rust-engineer-bg-v9.png`，Android 同步使用
  `embedded_rust_engineer_bg_v9.png`；两份 PNG 的 SHA-256 为
  `98E7CD8F291DE1EC2708736B963883CD9C7111CE349F1825C1D40FD8789C54B5`。
- v9 在保留左侧正文安全区的前提下，增强了角色的温和御姐表情、银色 Pro 形态笔记本、紧凑银色桌面
  工作站、代码/RL telemetry、示波器和开发板的层次；素材不承载可读业务文案、logo 或真实状态。
- v8 文件不覆盖、不删除，继续作为视觉回滚路径；生成文件已完成 Web/Android 双端目视检查，
  发布审计会比较两端 SHA-256 并校验当前引用。

## v8 受保护页面隔离预览（非合法账户证据）

- 本轮使用忽略目录 `windows/.cache/ui-audit-v8/` 的临时 SQLite schema 和短生命周期的预览身份，
  只用于渲染仪表盘、管理员页的空状态；没有读取或写入项目真实数据库、Key、Cookie 或 localStorage。
- 1440/768/320 的 dashboard 与 admin 观察样本已生成：
  [`dashboard-v8-1440.png`](../.cache/ui-audit-v8/dashboard-v8-1440.png)、
  [`dashboard-v8-768.png`](../.cache/ui-audit-v8/dashboard-v8-768.png)、
  [`admin-v8-1440.png`](../.cache/ui-audit-v8/admin-v8-1440.png) 和
  [`admin-v8-768.png`](../.cache/ui-audit-v8/admin-v8-768.png)。
- 预览确认空图表、空成员表和窄屏指标仍有可读内容；管理员成员详情新增 token 记录/工作事件
  空态，`data-reveal` 在脚本失败时默认保持可见，只有动效模块成功加载后才启用初始低透明度。
- 该预览不等同于真实认证会话、真实设备指标或最终 UI-3 通过证据；合法会话下的焦点、错误态、
  导出和 API 失败路径仍需独立复核。

## v9 场景合成与首帧可读性

- 当前源码进程使用显式装饰性 `<img>` 承载 v9 主图，CSS 背景声明保留为回退；场景层使用
  `z-index: 0`，内容/页脚使用 `z-index: 1`，避免固定负层级在 Chromium 中偶发消失。
- 登录入场动画改为轻微 opacity 变化并保留可见正文，移动端背景透明度和遮罩梯度收敛，
  防止首屏文字在动效尚未完成时显得像禁用状态。
- 本机当前源码服务的只读 Chrome 观察截图：
  [`login-v9-current-5017-1440.png`](../.cache/ui-audit-v9/login-v9-current-5017-1440.png)、
  [`login-v9-current-768.png`](../.cache/ui-audit-v9/login-v9-current-768.png) 和
  [`login-v9-current-320.png`](../.cache/ui-audit-v9/login-v9-current-320.png)。截图只证明
  当前源码渲染观察，不替代真实设备指标、合法会话或完整 UI-3 证据。
- v9 静态回归通过：模板渲染、前端样式加载、素材 200/PNG 响应、现有脚本语法和只读审计；
  Chrome 的 USB/Web App 安装提示与页面无关，不计为应用 console 错误。

## 尚未关闭的门禁

此前本证据只覆盖无需创建账户的登录运行时；v15/v16/v17 已补充隔离库内真实注册/登录后的
Dashboard/Admin 空态、动态错误播报、四档 viewport 和焦点回流观察。连接 Provider、历史写入后的
图表/记录、导出下载、真实设备指标、实际 reduced-motion/高对比度环境仍需分别复核，不能把隔离
viewport 观察扩展为生产或真实手机通过证据。UI-3 总闸门因此继续保持进行中。

## v10 场景可见度构图（v9 资产阶段）

- 当时默认场景使用 v9 同 SHA-256 资产；本轮只调整桌面遮罩和亮度，将右侧角色、Pro
  笔记本、工作站与示波器从低可见轮廓提升为可识别背景，同时保留左侧正文安全区。
- 普通模式、`prefers-contrast: more`、`forced-colors: active` 和 `prefers-reduced-motion`
  均保留独立降级路径；背景不参与语义、焦点或业务状态。
- 当前只完成临时源码服务的 1440px 登录页观察；不把它扩展为受保护页面或真实设备通过证据。

## v10 窄屏边界复核（当前记录）

- 当前源码在隔离端口 `5019` 和临时 SQLite 下重新渲染；未读取真实数据库、Cookie、Key 或令牌。
- `ui-audit-v10/login-v10-1440.png` 是 v8 阶段的历史截图，确认当时场景、正文层、认证卡片和焦点样式可读；
  当前默认素材由 v9 资产段落单独记录。
- 追加了窄屏 grid track、表单控件 `min-width: 0`、`100vw` 上限和场景签名换行约束，降低真实 320px 设备发生 intrinsic-width 裁切的风险。
- Chrome headless 的 320px CLI 截图仍出现比截图画布更宽的布局轨道；该运行器未提供可靠的设备指标覆盖，因此只作为 renderer observation，不能宣称真实手机横向溢出已通过。UI-3 继续保持进行中，待可用 CDP/真实设备证据后复核。

## v11 注册页语义顺序与布局回归

- 注册页 DOM 顺序已调整为 `h1` 主标题在前、认证卡片 `h2` 在后；桌面端通过显式 grid placement 保持“卡片左、场景右”的原视觉构图，窄屏端按 DOM 顺序自然排列。
- 当前源码隔离服务生成了 [`register-v11-1440.png`](../.cache/ui-audit-v11/register-v11-1440.png) 和 [`register-v11-768.png`](../.cache/ui-audit-v11/register-v11-768.png)；1440 档布局和焦点样式可读，768 档仍受 headless 自动聚焦/滚动行为影响，只作为观察样本。
- 模板静态 heading 扫描结果：`register.html`、`login.html`、`dashboard.html`、`admin.html` 和 `privacy.html` 均以 `h1` 开始；表单 label、图片 alt、表格 caption/scope 继续通过静态检查。

## v12 认证反馈语义补强

- `base.html` 的 flash 错误现在使用原生 `role="alert"`，成功和提示使用 `role="status"` 与
  `aria-live="polite"`；颜色仍保留视觉区分，但屏幕阅读器不再需要依赖颜色或轮询 DOM 才能获知
  登录/注册失败。
- 该切片只证明模板语义和静态边界已补齐；受保护页面的真实错误态、键盘回归、真实设备指标和
  合法会话仍属于 UI-3 未关闭门禁。

## v13 系统对比度降级

- `prefers-contrast: more` 会提升辅助文字和边框 token、加深内容面板并降低背景图透明度；
  `forced-colors: active` 会移除背景/鼠标装饰，交给 Windows 系统色处理内容、控件和焦点。
- 该规则不改变默认桌面构图，也不把动画作为可读信息来源；真实系统高对比度设备仍需在 UI-3
  合法会话验收中复核。

## v14 Dashboard 标题语义修正

- 受保护 Dashboard 的视觉标题包含 glitch 伪元素；此前浏览器无障碍树可能把装饰性副本重复读出。
- 当前 `h1` 使用稳定的 `aria-label="TOKEN SIGNAL"`，两层视觉字形标记为 `aria-hidden="true"`，
  保留视觉动画但只向辅助技术暴露一次标题。

## v15 隔离合法会话运行时回归

- 使用忽略目录 `windows/.cache/ui-audit-v12/` 的隔离 SQLite，在短生命周期 `5019` 端口通过真实
  注册/登录流程创建演示账户；随后只在该隔离库中授予 `admin` 角色，未读取或写入真实数据库。
- Dashboard 运行时无障碍树将标题暴露为一次 `TOKEN SIGNAL`；320px viewport 的
  `body/documentElement.scrollWidth=305`、`innerWidth=320`，无文档级横向溢出。
- Admin 运行时包含团队表、token 明细表和工作事件表；三张表均有 caption 和 scoped headers。
  成员详情展开后焦点进入关闭按钮，关闭后焦点回到成员“查看”触发按钮；空 token/事件状态均有可读文案。
- 768px Admin viewport 的 `documentElement.scrollWidth=753`、`innerWidth=768`，表格在自身
  `.table-wrap` 内水平滚动，页面本身没有横向溢出；本次浏览器 error/warning 日志为空。
- 该证据覆盖了受保护页面的真实会话、空态、焦点和两档显式 viewport；仍不等同于真实手机硬件指标，
  Provider 成功/失败、导出下载、reduced-motion 和完整四档焦点回归继续保持 UI-3 未关闭门禁。

## v16 动态错误播报与恢复回归

- Dashboard 在隔离合法会话下点击“自动检测模型”但不提交 Key，错误消息与 Provider 状态均切换为
  `role="alert"`、`aria-live="assertive"`、`aria-atomic="true"`；页面无 console error/warning。
- 切换统计周期并成功刷新后，Dashboard 状态恢复为 `role="status"`、`aria-live="polite"`，错误样式被清除，
  证明错误语义不会污染后续正常状态。
- Admin 明细加载保持 `role="status"`、`aria-live="polite"`，焦点进入关闭按钮；新增 `live-region.js`
  统一 Dashboard/Admin/Provider 的动态播报边界，详见 ADR-067。
- 本切片仍未宣称 Provider 网络失败、导出下载、真实设备 reduced-motion 和完整四档焦点门禁通过。

## v17 四档受保护页面回归

- 使用同一隔离 SQLite 和短生命周期 `5019` 源服务，将 Dashboard/Admin 分别置于 320、768、1024、
  1440 viewport；页面级 `scrollWidth` 均不超过 `clientWidth`：对应有效内容宽度为
  `305/753/1009/1425`，四档均无横向溢出。
- 四档 Dashboard 均只有一个 `h1`；Admin 四档均渲染 3 张表、3 个 caption，表格溢出保持在表格容器内。
- Dashboard 四档 Tab 顺序均从跳过链接、品牌、导航/退出、主操作进入周期控件；320 档导航按窄屏规则隐藏，
  不产生不可达的空焦点目标。
- Admin 四档打开成员明细后焦点均进入 `admin-detail-close`，关闭后均回到原“查看”按钮；本次应用页面
  `error/warning` 日志为空，viewport override 已在结束时重置。
- 该证据覆盖隔离浏览器 viewport，不等同于真实设备指标；Provider 网络失败、导出下载、真实
  reduced-motion/高对比度系统环境和历史非空数据仍保持未关闭门禁。

## v18 连接失败、历史非空与导出回归

- 使用同一忽略目录下的隔离 SQLite 和短生命周期 `5019` 源服务，通过合法隔离会话在真实页面提交
  一条补录记录；页面收到 `POST /api/records` `201`，历史表显示 `kimi-code`、输入 `1,200`、输出
  `350`、合计 `1,550`，并保留备注。该记录只存在于隔离库，不是项目测试数据或生产数据。
- 切换到 `Week` 后，历史记录仍可见，导出链接同步为 `/api/export?period=week`，Dashboard 状态恢复为
  `role=status`、`aria-live=polite`。应用访问日志记录 CSV 请求 HTTP `200`；独立浏览器随后将下载动作
  拦截为 `ERR_BLOCKED_BY_CLIENT`，因此本证据证明服务端导出响应，不宣称浏览器下载落盘已通过。
- 使用合成无效 Key 对白名单 OpenAI-compatible 地址执行只读模型检测；服务端记录
  `POST /api/provider/models` HTTP `502`，页面显示“检测失败”，Provider 状态和错误消息均切换为
  `role=alert`、`aria-live=assertive`、`aria-atomic=true`，检测按钮最终恢复可用。
- 本轮应用页面 DevTools `error/warning` 日志为空；浏览器工具自身的外部 Statsig 超时不属于应用页面日志，
  不计入应用失败。真实 Provider 成功、真实设备下载、系统级 reduced-motion/高对比度仍需后续门禁。

## v19 个人工作信号竖切片回归

- 使用同一忽略目录下的隔离 SQLite、`5019` 源服务和合法隔离会话打开 Dashboard；页面请求
  `GET /api/v1/events/work` 返回 `200`，空状态显示“已加载 0 条工作事件”。
- 通过真实网页表单提交方向、结果、效率 `91`、项目、任务类型、正确码和结构化备注；服务端记录
  `POST /api/v1/events/work` `201`，随后列表 `GET` `200`，页面显示“工作信号已保存”和最近活动行。
  表格使用 `textContent` 渲染，未把输入内容当作 HTML。
- 使用含空格的非法错误码验证服务端校验；服务端返回 `400`，页面消息切换为
  `role=alert`、`aria-live=assertive`，卡片进入 error 状态且提交按钮恢复可用。清除错误值后再次提交，
  服务端返回 `201`，列表显示两条事件，消息恢复为 `role=status`、`aria-live=polite`，卡片回到 success 状态。
- 当前默认浏览器视口的 `documentElement.scrollWidth - clientWidth = 0`；Activity 双栏在 `900px` 以下
  堆叠，历史表格自身保留横向滚动。应用页面 DevTools `error/warning` 日志为空；本轮服务端访问日志同时
  记录了 `GET 200`、`POST 201` 和预期的验证 `POST 400`。
- 该证据只覆盖隔离合法会话下的功能/状态切片，不宣称真实手机四档指标、Provider 真实成功、下载落盘、
  系统级 reduced-motion/高对比度或正式部署数据保留门禁已关闭。

## v20 Admin 成员脱敏日志详情回归

- 在忽略目录 `windows/.cache/ui-audit-v12/ui3-20260810.db` 中写入一条无敏感内容的结构化诊断日志，
  通过真实管理员页面选择成员；`GET /api/v1/admin/users/<id>/records` 返回的 `logs` 投影被页面渲染到
  `Diagnostic logs` 表格，显示时间、等级、事件、错误码、消息和 request id。
- 日志表 caption 为“选中成员的脱敏诊断日志”，日志单元格使用 `textContent`；成员详情打开后焦点进入
  `admin-detail-close`，页面级横向溢出为 `0`，长表格保持在自身容器边界内。
- 本轮应用页面 DevTools `error/warning` 日志为空。该证据不替代真实生产日志保留、ACL、导出审计和真实
  设备四档 UI-3 门禁。

## v21 v9 场景背景可见度调校

- 在临时源码实例 `127.0.0.1:5019` 和隔离 SQLite 中打开登录页；未读取真实数据库、Cookie、Key 或令牌。
- 运行时截图确认右侧御姐二次元嵌入式工程师、笔记本、银色桌面工作站和示波器已从暗纹理提升为
  可辨识场景，左侧 `SEE THE SIGNAL` 标题、中文说明和登录卡片仍保持独立高对比内容层。
- DOM 只读检查确认 v9 PNG `naturalWidth/naturalHeight` 有效，`story-backdrop` 的滤镜为
  `saturate(1.18) contrast(1.08) brightness(1.1)`，`backdrop-glow`/`backdrop-scan` 仍为 16s/13s，
  页面没有新增横向溢出。
- 本轮只覆盖临时源码实例的桌面登录页；真实 320px 设备、系统 reduced-motion/高对比度、受保护页和
  Android 硬件仍保持 UI-3 未关闭，不把截图扩展为正式发布证据。

## v22 v10 场景资产切换（当前）

- 当前默认场景已切换到 `embedded-rust-engineer-bg-v10.png`，Android 同步使用
  `embedded_rust_engineer_bg_v10.png`；两端 SHA-256 均为
  `04DBB4FD9F57CBAAA9D5D719D6B562AD4F22B13AF87691638B6B370E8AB2FB7E`。
- v10 保留左侧深色文案安全区，强化右侧成年二次元嵌入式工程师、Rust/RL 代码与训练曲线、银色
  专业笔记本、银色紧凑桌面工作站、示波器和实验板的辨识度；图片不承载业务文案、Logo 或水印。
- v9/v8 继续保留为回滚资产；此次切换只更新 Web/Android 静态资源引用，不改变业务 API、认证、
  数据库或动效契约。真实设备、系统级 reduced-motion/高对比度和 Android 编译仍待对应门禁。

## v23 认证页 glitch 标题语义层修复

- 在当前 checkout 启动的隔离源码实例 `127.0.0.1:5019` 中复核登录/注册页；数据库位于临时目录，
  未读取真实数据库、Cookie、Key 或令牌，验证结束后临时进程已停止且临时 SQLite 已删除。
- 发现并修复认证页 `h1` 的 CSS generated content 可能被无障碍树重复观察的问题：语义层改为唯一的
  `sr-only` `h1`，视觉层使用相邻 `aria-hidden` 容器承载分行字形和 glitch 伪元素。登录页
  `getByRole('heading', name="SEE THE SIGNAL.")`、注册页 `getByRole('heading', name="TRACK THE FLOW.")`
  均返回 1 个，且每页只有 1 个 `h1`。
- 当前源码实例截图确认注册页 v10 角色、笔记本、银色桌面工作站和示波器仍可见；键盘顺序为用户名、
  密码、提交、页内链接、隐私链接；登录/注册页面级 `scrollWidth` 等于 `clientWidth`，应用控制台
  error/warning 为空。
- 该切片只关闭认证标题重复语义风险；真实系统 `prefers-reduced-motion`、高对比度/forced-colors、
  真实设备指标、下载落盘和 Provider 成功仍保持 UI-3 未关闭门禁。

## v24 Android 系统减少动画策略（源码交付）

- 新增 `android/app/src/main/java/com/aitokentracker/ui/MotionPreferences.kt`，统一读取
  `ANIMATOR_DURATION_SCALE` 与 `TRANSITION_ANIMATION_SCALE`；任一为 `0` 时返回减少动画状态。
- `TokenTrackerApp` 的品牌背景在该状态下使用 `StaticBrandBackdrop`，`SignalOrbit` 使用
  `StaticSignalOrbit`；动态分支才创建 `rememberInfiniteTransition`，静态分支不启动无限循环。
- 这一步是跨端源码和依赖方向证据，不冒充 Android 真机/模拟器运行证据；JDK、Gradle、SDK、系统
  设置和帧耗时仍需获批工具链完成。

## v25 v11 场景背景资产与可读性升级

- 当前默认场景已切换到 `embedded-rust-engineer-bg-v11.png`，Android 同步使用
  `embedded_rust_engineer_bg_v11.png`；两端 SHA-256 均为
  `48BDB3616CA209F4B6412051F6018C3A47E944D612E966D146EEB4C1DBB7E6A`。
- v11 将成年御姐风嵌入式工程师、银色专业笔记本、独立紧凑桌面工作站和 Rust/RL 遥测屏幕收束到
  右侧视觉层，左侧继续保留低细节深海军蓝正文安全区；图片无 Logo、无可读文案、无水印。
- Web `scene-motion.css` 与 Android Compose 仅更新资源引用和装饰层亮度，不改变业务 API、认证、
  数据库、网络状态或动效语义；v10/v9/v8 继续作为回滚资产。
- 本轮已完成生成图静态检查、跨端字节哈希和源码契约更新；真实 320px/设备指标、系统
  reduced-motion/高对比度与 APK 视觉验收仍保持 UI-3/Android 条件门禁。
- 在隔离源码实例 `127.0.0.1:5019` 使用本机 Chrome headless 完成登录页首屏复核：
  [`login-v25-1440.png`](../.cache/ui-audit-v25/login-v25-1440.png) 确认左侧标题/正文安全区、右侧角色、
  银色专业笔记本、独立桌面工作站与诊断屏幕层级清晰；卡片边框、焦点输入框和背景遮罩没有互相吞字。
  [`login-v25-390.png`](../.cache/ui-audit-v25/login-v25-390.png) 覆盖窄屏裁切观察；截图触发了自动聚焦输入框的
  页面滚动，不能替代真实 390px 设备首屏与系统 reduced-motion 证据。
- 隔离实例只使用临时 SQLite，`GET /login` 返回 200；验证结束后仅停止本轮启动的 5019 进程，受保护的
  5000/5011 进程与监听状态未触碰。Chrome 自身 USB/Web App warning 不属于应用页面日志，不计入应用失败。

## v26 v14 浏览器运行首屏证据

- 在当前 checkout 启动隔离源码实例 `127.0.0.1:5024`，数据库位于
  `windows/.cache/ui-v14-browser-20260810/token_tracker.sqlite3`；未读取真实数据库、Cookie、Key 或令牌。
- 默认浏览器视口观测为 `innerWidth=1036`、`innerHeight=850`；页面
  `documentElement.scrollWidth=1021`，没有横向溢出。浏览器扩展没有应用请求的 390px/1440px viewport override，
  因此本轮不宣称 320/390/768/1024/1440 响应式证据。
- 登录页 DOM/CSS 实际引用 `/static/assets/embedded-rust-engineer-bg-v14.png`；焦点自动落在用户名输入框，
  页面包含唯一语义 `h1`、表单字段和可访问提交按钮。动效完成后，左侧标题/中文说明与右侧认证卡片、角色和设备场景均保持清晰分层。
- 当前页截图：[`login-v14-default.png`](../.cache/ui-v14-browser-20260810/login-v14-default.png)。浏览器
  DevTools `error/warning` 日志为空；性能 `performance` API 在该浏览器评估沙箱中不可用，未记录或推断性能指标。
- 隔离实例验证结束后仅停止本轮启动的 5024 进程，端口已释放；受保护的 5000/5011 进程与监听状态未触碰。
- 该证据关闭 v14 默认桌面首屏的源码运行观察，不替代真实 320/768/1024/1440、系统
  `prefers-reduced-motion`/高对比度、浏览器下载落盘和 Android 真机验收。

## v27 v14 四档浏览器运行证据

- 在当前 checkout 启动隔离源码实例 `127.0.0.1:5025`，数据库位于
  `windows/.cache/ui-v14-runtime-20260810/token_tracker.sqlite3`；未读取真实数据库、Cookie、Key 或令牌。
- Edge 实际浏览器成功应用 viewport override，并逐档重新加载登录页。四档结果如下：

  | 视口 | `scrollWidth` | `scrollHeight` | `h1` | `label` | 焦点 |
  | --- | ---: | ---: | ---: | ---: | --- |
  | 320×800 | 305 | 1337 | 1 | 2 | `username` |
  | 768×1024 | 753 | 1325 | 1 | 2 | `username` |
  | 1024×768 | 1009 | 959 | 1 | 2 | `username` |
  | 1440×900 | 1425 | 1091 | 1 | 2 | `username` |

  每一档均满足 `scrollWidth <= innerWidth`，没有页面级横向溢出；`scrollHeight` 大于视口高度是登录内容
  在窄屏纵向排列的预期结果。补充的 390×844 观察为 `scrollWidth=375`、`scrollHeight=1337`。
- 每一档均存在可用登录按钮，DOM snapshot 只有一个语义 `h1`，用户名/密码字段均有 label，焦点自动回到
  用户名输入框。移动端和桌面端截图均观察到 v14 角色、笔记本、桌面工作站/RL 屏幕、标题和认证卡片
  保持分层；没有遮挡正文的异常。
- DevTools `error/warning` 日志为空。只读计算样式观察到标题颜色为 `rgb(247, 248, 252)`、label 为
  `rgb(251, 249, 255)`、正文为 `rgb(215, 219, 229)`，输入文字为 `rgb(247, 248, 252)`，提交按钮为
  `rgb(217, 255, 120)` 背景配 `rgb(17, 21, 11)` 文字；本轮未推断未测量的对比度比例。
- 证据完成后已重置 viewport override，恢复默认浏览器尺寸，并只停止本轮启动的 5025 进程；受保护的
  5000/5011 进程与监听状态未触碰。
- 该证据强化了当前 v14 登录页的浏览器四档运行观察，不替代真实手机系统指标、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v28 登录认证卡片玻璃层与信号轨道源码切片（2026-08-11）

- `token_tracker/static/ui-polish.css` 将登录/注册认证卡片收敛为半透明玻璃层：渐变表面透明度由近实心层
  调整为 `rgba(23,26,33,.62)` / `rgba(10,12,16,.72)`，背景模糊提升到 `18px`，并保留独立的可见边框、
  `focus-within` 高亮、顶部信号轨道和状态点。
- 输入控件继续使用独立表面与焦点环，不依赖卡片底色承载文字对比度；`prefers-reduced-motion` 会停用信号轨道，
  `forced-colors` 会隐藏装饰轨道并恢复系统 Canvas/Highlight 语义。
- 本轮完成 `git diff --check` 与 `token_tracker audit --json`：`14 pass / 1 pending / 0 fail`。本轮只记录源码与静态
  门禁结果，不把既有 v27 截图重新标记为 v28 运行证据；v27 仍是最近一次浏览器四档运行证据。
- 本切片不改变认证、API、数据库、Provider 或部署边界；真实设备、系统 reduced-motion/高对比度、浏览器下载落盘、
  真实 Provider 和 Android 真机门禁保持原状态。

## v29 场景左上遮罩与导航玻璃层运行证据（2026-08-11）

- 在当前 checkout 启动隔离源码实例 `127.0.0.1:5026`，数据库位于
  `windows/.cache/ui-v1000-runtime-20260811/token_tracker.sqlite3`；未读取真实数据库、Cookie、Key 或令牌。
- `scene-motion.css` 将桌面左侧遮罩从 `0.985` 起步的近实心黑层调整为分段半透明 veil，并同步降低 900px/620px
  断点的遮罩强度；顶部左侧新增低强度蓝色环境光。`ui-polish.css` 将固定导航表面调整为 `rgba(8,9,12,.58)`
  并保留 `backdrop-filter`，使上方场景连续透出。
- 浏览器截图观察到左上网格、Rust/RL 工作站、角色、MacBook、Mac Studio 和诊断屏幕均可辨识，正文与认证卡片
  仍保持独立内容层。默认运行时确认背景图已加载（`naturalWidth=1672`），计算样式中的左侧 veil 与导航半透明值
  与源码一致。
- Edge 四档 viewport 结果如下；每档均满足 `scrollWidth <= clientWidth`，唯一语义 `h1`、两个 `label`、提交按钮和
  `username` 焦点均存在：

  | 视口 | `scrollWidth` | `scrollHeight` | `h1` | `label` | 提交按钮 | 焦点 |
  | --- | ---: | ---: | ---: | ---: | --- | --- |
  | 320×800 | 305 | 1350 | 1 | 2 | 是 | `username` |
  | 768×1024 | 753 | 1338 | 1 | 2 | 是 | `username` |
  | 1024×768 | 1009 | 959 | 1 | 2 | 是 | `username` |
  | 1440×900 | 1425 | 1091 | 1 | 2 | 是 | `username` |

- 浏览器控制台 `error/warning` 为空；本轮只停止隔离端口 5026 进程，受保护的 5000/5011 进程与监听状态未触碰。
- 该证据关闭 v29 场景遮罩源码与隔离浏览器运行观察，不替代真实设备、系统 `prefers-reduced-motion`/高对比度、
  浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v30 Dashboard 图表空态行动入口与非空回归证据（2026-08-11）

- 在当前 checkout 启动隔离源码实例 `127.0.0.1:5027`，数据库位于
  `windows/.cache/ui-v1001-runtime-20260811/token_tracker.sqlite3`；使用临时 `uiobserver` 账号验证，未读取真实数据库、
  Cookie、Key 或令牌。
- `static/modules/charts.js` 的空态渲染器新增可访问的 `chart-empty-action` 锚点，趋势图和模型占比都指向
  `#auto-entry`；文案使用 `textContent`/节点追加，未引入 HTML 字符串渲染。`ui-polish.css` 为该入口补充紧凑的
  lime signal pill、hover/focus-visible 和箭头过渡。
- 空账户运行时确认两个图表均为 `empty`，`chart-empty-action` 数量为 2；点击第一个入口后 URL hash 为
  `#auto-entry`，等待平滑滚动结束时 `#auto-entry` 距视口顶部约 `-4px`。
- 隔离账户通过页面“异常补录”写入一条 `kimi-code` 的 `1200 + 350 = 1550` token 样例后，两个图表均为 `ready`，
  CTA 数量归零；趋势图和模型占比截图均正常渲染，最近记录显示该样例。该样例仅存在于临时数据库。
- 四档响应式复核结果如下；每档均满足 `scrollWidth <= clientWidth`，Dashboard 保持唯一 `h1`，自动采集区存在；
  320px 导航自动隐藏，其余视口导航保持可用：

  | 视口 | `scrollWidth` | `clientWidth` | `h1` | 空态 CTA | 自动采集区 | 导航 |
  | --- | ---: | ---: | ---: | ---: | --- | --- |
  | 320×800 | 305 | 305 | 1 | 2 | 存在 | 隐藏 |
  | 768×1024 | 753 | 753 | 1 | 2 | 存在 | 可见 |
  | 1024×768 | 1009 | 1009 | 1 | 2 | 存在 | 可见 |
  | 1440×900 | 1425 | 1425 | 1 | 2 | 存在 | 可见 |

- 空态、补录成功、非空图表和响应式复核后的浏览器控制台 `error/warning` 均为空；本轮只停止隔离端口 5027 进程，
  受保护的 5000/5011 进程与监听状态未触碰。
- 该证据关闭 v30 空态行动入口与非空图表回归观察，不替代真实设备、系统 `prefers-reduced-motion`/高对比度、
  浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v31 左上场景首帧与管理员观测台运行证据（2026-08-11）

- 在当前 checkout 启动隔离源码实例 `127.0.0.1:5028`，数据库位于
  `windows/.cache/ui-v1001-runtime-20260811/token_tracker.sqlite3`；使用临时管理员会话观察，未读取真实数据库、
  Cookie、Key 或令牌。
- `scene-motion.css` 将左侧 veil 收敛为冷色环境光加轻遮罩，并提升场景亮度；900px/620px 断点同步降低黑色层，
  移动端背景保留 `0.9` 透明度。`style.css` 将全局首帧从 `opacity: 0` 改为 `0.84` 起步，避免登录页在图片/字体
  稳定前被误认为黑色覆盖层。浏览器首帧观察到 `body opacity≈0.964`，稳定帧左上服务器、代码屏幕、角色和设备均可辨识。
- `ui-polish.css` 为 Admin People/Export 数据轨道增加紧凑状态胶囊、用户行 signal 点、导出层级和窄屏安全边界；
  Admin 详情按钮仍保持 `aria-expanded`，打开后焦点回流到 `#admin-detail-close`。
- 登录页与管理员页四档 viewport 均无页面级横向溢出：

  | 页面 | 320 | 768 | 1024 | 1440 |
  | --- | ---: | ---: | ---: | ---: |
  | 登录 `scrollWidth/clientWidth` | 305/305 | 753/753 | 1009/1009 | 1425/1425 |
  | Admin `scrollWidth/clientWidth` | 305/305 | 753/753 | 1009/1009 | 1425/1425 |

- Admin 隔离会话在四档均返回 `LIVE / AUDITED`；成员详情打开后 `hidden=false`、触发器为
  `aria-expanded=true`、活动焦点为 `admin-detail-close`。浏览器控制台 `error/warning` 为空。
- 验证结束后仅停止隔离端口 5028；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、系统
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v32 Admin 统一动效契约与动态详情可读性证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5030`，数据库位于
  `windows/.cache/ui-v1003-runtime-20260811/token_tracker.sqlite3`；临时账户仅用于浏览器观察，未读取真实
  数据库、Cookie、Key 或令牌。
- `admin.js` 接入共享的 `setupReveal`、`setupPointerFollower`、`setupBackdropMotion` 和
  `setupSurfaceMotion`；Admin 桌面端指针移动后观察到 `pointer-ready=true`、`pointer-interactive=true`，
  aura/ring 透明度均为 `1`。
- `motion.js` 对初始化时 `hidden` 的 `data-reveal` 节点直接标记 `is-visible`。Admin 成员详情
  打开前 `hidden=true / opacity=1`，点击“查看”后 `hidden=false / opacity=1`，焦点为
  `admin-detail-close`，触发器 `aria-expanded=true`。
- 320×800 移动视口确认 `scrollWidth=305 / clientWidth=305`、导航隐藏、详情隐藏节点仍保持
  `opacity=1`；背景层透明度为 `0.9`。浏览器控制台 `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5030；5000/5011 仍由原有进程监听。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v33 顶部玻璃导航与滚动章节状态证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5033`，数据库位于
  `windows/.cache/ui-v1005-runtime-20260811/token_tracker.sqlite3`；临时账户仅用于浏览器观察，未读取真实
  数据库、Cookie、Key 或令牌。
- 新增 `static/modules/navigation.js`，由 Dashboard/Admin 页面编排层调用；Admin 桌面端导航将
  `Admin` 标记为可见 active 状态并设置 `aria-current="location"`，Dashboard 首屏将 `Analysis` 标记为
  active，点击/滚动到 `Connect`、`Activity` 后分别更新 hash 与当前位置高亮。
- 1440×900 桌面观察到顶部滚动进度线随页面滚动从 `0.00%` 更新到 `99.93%`；Dashboard 到达
  `#activity` 时观察到 `aria-current="location"` 与 `Activity` 高亮同步，进度为 `38.29%`。
- 顶部 `.site-header` 改为低不透明度渐变玻璃层，运行时确认背景为半透明渐变、
  `backdrop-filter: blur(14px) saturate(1.16)`；截图中品牌背景与场景图可见，AI TOKEN、导航和退出按钮仍保持清晰。
- 320×800 移动视口确认 `scrollWidth=305 / clientWidth=305`、导航按预期隐藏、Dashboard 默认
  `Analysis` 状态保留、顶部玻璃层仍启用；页面控制台 `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5033；5000/5011 仍由原有进程监听。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v34 自动采集双栏表面高度证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5041`，数据库位于
  `windows/.cache/ui-v1006-runtime-20260811/token_tracker-5041.sqlite3`；临时账户仅用于浏览器观察，未读取真实
  数据库、Cookie、Key 或令牌。
- `tool-grid.auto-layout` 改为 `align-items: start`，说明卡片显式使用 `align-self: start`；桌面运行时
  左侧自动采集表单约 `884px`，右侧 `HOW IT WORKS` 说明卡片约 `520px`，卡片在说明内容结束处收口，
  不再出现被长表单撑出的空黑区域。
- 320×800 移动视口确认自动采集布局退化为单列、导航隐藏，`scrollWidth=305 / clientWidth=305`；
  浏览器控制台 `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5041；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v35 待显区块可读性与动效收束证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5042`，数据库位于
  `windows/.cache/ui-v1007-runtime-20260811/token_tracker-5042.sqlite3`；临时账户仅用于浏览器观察，未读取真实
  数据库、Cookie、Key 或令牌。
- `ui-polish.css` 将 `html.motion-ready [data-reveal]` 的待显状态从 `opacity: .16 / translateY(18px)`
  收敛为 `opacity: .34 / translateY(12px)`；桌面长页面中下方表单、活动和历史区块保持可辨识的层次，
  不再像被黑色遮罩覆盖。通过 `#activity` 锚点进入视口后，运行时确认区块最终 `opacity=1`、位移归零。
- 320×800 移动视口确认 `scrollWidth=305 / clientWidth=305`、导航隐藏，待显区块仍为 `opacity=.34`；
  浏览器控制台 `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5042；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v36 表格零信号空态证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5043`，数据库位于
  `windows/.cache/ui-v1008-runtime-20260811/token_tracker-5043.sqlite3`；临时账户仅用于浏览器观察，未读取真实
  数据库、Cookie、Key 或令牌。
- Dashboard 的 Token 历史表、Activity 工作事件表和 Admin 空表行统一增加
  `table-empty--signal`；空状态包含结构化中心信标、细扫描线、可读说明和受 `prefers-reduced-motion` 约束的
  微动效，加载态仍保持纯文本，不把加载误示为无数据。
- 桌面运行时 Activity 空态单元格约 `549×136px`，Token 历史空态约 `1269×136px`；320×800 移动视口
  确认空态仍在表格内部滚动容器中，页面 `scrollWidth=305 / clientWidth=305`、导航隐藏，浏览器控制台
  `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5043；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v37 周期切换 active pill 与响应式证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5044`，数据库位于
  `windows/.cache/ui-v1009-runtime-20260811/token_tracker-5044.sqlite3`；临时账户仅用于浏览器观察，未读取真实
  数据库、Cookie、Key 或令牌。
- 新增 `static/modules/range-switcher.js`，周期按钮统一同步 `aria-pressed`、active class 和 CSS 自定义属性；
  桌面端切换 Today/Week/Month/All time 后，状态分别更新为今天/本周/本月/全部，active pill 的宽度与 X 位移
  与当前按钮几何一致，切换过程使用连续 transform/width 过渡。
- 320×800 移动视口确认周期切换器仍在容器内，点击 Month 后 `rangeX` 与按钮实际位移一致，
  `scrollWidth=305 / clientWidth=305`、导航隐藏；浏览器控制台 `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5044；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v38 表单选择控件暗色玻璃表面证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5045`，数据库位于
  `windows/.cache/ui-v1010-runtime-20260811/token_tracker-5045.sqlite3`；临时账户仅用于浏览器观察，未读取真实
  数据库、Cookie、Key 或令牌。
- 工作信号表单的“工作方向/结果”和自动采集的 Provider 预设统一使用暗色玻璃 select：
  `appearance=none`、`color-scheme=dark`、浅色文字、自定义箭头、聚焦边框与暗色 option 背景，消除原生白色控件
  与观测台表面的视觉断层。
- 桌面运行时两个工作信号 select 均为 `48px` 高、文字色为 `rgb(247,248,252)`；自动采集预设同步保留箭头。
  320×800 移动视口下 select 宽 `237px`，页面 `scrollWidth=305 / clientWidth=305`，实际选择“调试排错”
  成功更新值；浏览器控制台 `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5045；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v39 顶部 AI TOKEN 透明玻璃栏证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5046`，数据库位于
  `.cache/ui-v1011-runtime-20260811/token_tracker-5046.sqlite3`；临时账户仅用于浏览器观察，未读取真实数据库、
  Cookie、Key 或令牌。
- `static/ui-polish.css` 将顶部 `.site-header` 的深色渐变遮罩从 `rgba(8,9,12,.34)` 收敛为
  `rgba(8,9,12,.08) → transparent`，保留 `blur(8px) saturate(1.08)` 作为轻量透景层，并以文本阴影维持
  `AI TOKEN`、导航和账户操作在插画背景上的可读性；滚动到 `#connect` 后 sticky header 仍保持同一透明层。
- 桌面运行时确认 header 背景为低不透明度渐变，`backdrop-filter=blur(8px) saturate(1.08)`，滚动后
  `headerTop=0`；320×800 移动视口确认品牌仍可见、导航按预期隐藏、`scrollWidth=305 / clientWidth=305`。
  浏览器控制台 `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5046；受保护的 5000/5011 进程仍分别由 PID 43832/8100 监听。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v40 异常补录 disclosure 状态证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5047`，数据库位于
  `.cache/ui-v1012-runtime-20260811/token_tracker-5047.sqlite3`；临时账户仅用于浏览器观察，未读取真实数据库、
  Cookie、Key 或令牌。
- `#manual-entry` 继续使用原生 `<details>/<summary>`，视觉层新增收起/hover/focus/open 四种状态：打开时边框为
  `rgba(217,255,120,.46)`、顶部信号线展开、summary 进入浅绿色渐变、图标旋转为关闭态，`.manual-details-body`
  使用 `manual-details-in` 内容渐入；图标标记为 `aria-hidden=true`，不干扰折叠区语义。
- 桌面运行时确认初始 `open=false`；点击后 `open=true`、`bodyOpacity=1`、`bodyTransform=none`，使用 `Space`
  关闭、`Enter` 再打开时焦点仍停留在 summary。320×800 移动视口确认 summary 高 `84px`、折叠区宽约
  `274.67px`、`scrollWidth=305 / clientWidth=305`、导航隐藏；展开后边框状态与桌面一致，浏览器控制台
  `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5047；受保护的 5000/5011 进程仍分别由 PID 43832/8100 监听。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v41 章节锚点与 sticky header 安全落点证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5048`，数据库位于
  `.cache/ui-v1013-runtime-20260811/token_tracker-5048.sqlite3`；临时账户仅用于浏览器观察，未读取真实数据库、
  Cookie、Key 或令牌。
- `static/ui-polish.css` 为 `#analysis/#connect/#activity/#history` 统一增加
  `scroll-margin-top: clamp(84px, 7vw, 104px)`；修改前 `Connect` 标题被 sticky header 遮住，修改后桌面端
  `Analysis/Connect/Activity` 的 section top 均约 `104px`，顶部栏底线为 `76px`，标题和表单完整可见。
- 320×800 移动视口下 `Connect` section top 约 `84px`，顶部栏底线为 `76px`，导航隐藏，页面保持
  `scrollWidth=305 / clientWidth=305`；`History` 在页面最大滚动边界处遵守内容高度约束，没有强行制造底部空白。
  页面控制台中属于本地应用的 `error/warning` 为空。
- 验证结束后仅停止显式隔离端口 5048；受保护的 5000/5011 进程仍分别由 PID 43832/8100 监听。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v42 数据面板阅读遮罩与响应式证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5050`，数据库位于
  `.cache/ui-v1014-runtime-20260811-5050/token_tracker-5050.sqlite3`；临时账户仅用于浏览器观察，未读取真实数据库、
  Cookie、Key 或令牌。另以隔离账号复核无记录空态，以一条临时 `kimi-code` 记录复核非空趋势/模型占比。
- `static/ui-polish.css` 为图表、Token 历史和工作事件卡片增加更稳定的深色阅读遮罩：插画仍保留在卡片边缘作为场景层，
  但不再穿透图表坐标轴、表头、空态说明和表单标题。顶部 `.site-header` 继续保持透明渐变层，未回退 v39 的透景效果。
- 桌面运行时确认 `#analysis` 锚点落点约 `103.72px`，两个非空图表状态均为 `ready`，数据卡片背景计算样式包含
  `rgba(15,19,25,.985) / rgba(7,10,14,.995)` 阅读遮罩，页面 `scrollWidth=1668 / clientWidth=1668`；空账号两个图表状态均为
  `empty`，CTA 和说明文字仍在 DOM/可访问状态中。
- 320×800 移动视口确认 `#analysis` top 约 `84.11px`、顶部栏底线约 `76px`、导航隐藏，页面保持
  `scrollWidth=305 / clientWidth=305`；空态与非空图表均可读，浏览器本地应用控制台日志为空。
- 验证结束后仅停止显式隔离端口 5049/5050；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v43 Token 历史层级、移动卡片化与透明品牌栏证据（2026-08-11）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5053`，数据库位于
  `.cache/ui-v1015-runtime-20260811-5053/token_tracker-5053.sqlite3`；临时账号仅用于浏览器观察，未读取真实数据库、
  Cookie、Key 或令牌。以三条补录记录复核非空历史，以一名新账号复核空态。
- `static/app.js` 为记录单元补充稳定的 `record-row`/`record-empty-row`、列 `data-label`、模型/总 token/来源语义 class；
  来源文案拆成安全的 `textContent` span，桌面端将模型作为行入口、总 token 作为主指标、自动采集/补录作为来源徽标。
  `static/ui-polish.css` 将顶部 `.site-header` 改为真正的透明玻璃层，仅保留轻量 blur、底部细线和文字阴影，避免 `AI TOKEN`
  品牌栏形成黑色横幅。
- 桌面非空历史截图确认模型标记、lime 总 token、来源 pill 与既有导出入口共同呈现；320×800 下每条记录转为完整双列
  卡片，`historyTop≈84.03px`、header bottom `76px`、`scrollWidth=305 / clientWidth=305`，完整展示模型、输入、输出、
  总 token、时间与来源备注，无横向滚动条。
- 320×800 空账号复核 `record-empty-row` 与原有空态信标仍存在，`scrollWidth=305 / clientWidth=305`；本地应用
  浏览器控制台日志为空。短内容页面的桌面/空态锚点若受最大滚动边界限制，按既有 v41 内容高度约束处理。
- 验证结束后仅停止显式隔离端口 5051/5052/5053；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v44 sticky header 自适应透景证据（2026-08-12）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5054`，数据库位于
  `.cache/ui-v1016-runtime-20260812-5054/token_tracker-5054.sqlite3`；临时账号仅用于本地浏览器观察，未读取真实数据库、
  Cookie、Key 或令牌。
- `static/modules/navigation.js` 通过 requestAnimationFrame 复用滚动进度更新，在 `scrollY > 16px` 时为 header 加入
  `is-scrolled`；`static/auth.js` 也复用同一导航上下文，因此登录页、注册页、Dashboard 和 Admin 的 sticky header 行为一致。
  首屏保持 `background-color: rgba(0,0,0,0)`，滚动后切换为 `rgba(8,9,12,.46)` 与更强 blur，阻止巨型 hero 字样透过导航形成白色噪声带。
- 桌面运行时确认初始视口 `1683×892`、页面 `scrollWidth=1668 / clientWidth=1668`、header 为透明背景和 `blur(8px)`；滚动后
  确认 `class=site-header is-scrolled`、背景 `rgba(8,9,12,.46)`。320×800 登录页确认首屏透明、滚动后自适应玻璃态，
  `scrollWidth=305 / clientWidth=305`，无横向溢出；本地应用控制台错误/警告为空。
- 验证结束后仅停止显式隔离端口 5054 并清理临时目录；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v45 透明品牌栏羽化渐变证据（2026-08-12）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5055`，数据库位于
  `.cache/ui-v1017-runtime-20260812-5055/token_tracker-5055.sqlite3`；临时账户仅用于本地浏览器观察，写入三条临时记录，未读取真实数据库、
  Cookie、Key 或令牌。
- `static/scene-motion.css` 将 `.site-header.is-scrolled` 从 `rgba(8,9,12,.46)` 深色实心阅读层收敛为透明背景和
  `linear-gradient(180deg, rgba(8,9,12,.14), transparent 88%)` 羽化渐变，并在滚动态关闭 `backdrop-filter`，避免白色
  `TOKEN/SIGNAL` 字样透过模糊层形成发白的横向雾带；首屏 `.site-header` 仍保持透明与原有低幅透景 blur。
- 桌面 `1683×892` 运行时确认首屏 `background=rgba(0,0,0,0)`；滚动 `420px` 后确认
  `class=site-header is-scrolled`、`background=rgba(0,0,0,0)`、羽化渐变生效、`backdrop-filter=none`，截图中 AI TOKEN
  品牌栏不再形成实心黑色横幅；页面 `scrollWidth=1668 / clientWidth=1668`。
- 320×800 移动视口确认首屏与滚动态均保持透明，滚动态过渡完成后 `backdrop-filter=none`，导航按既有策略隐藏，页面保持
  `scrollWidth=305 / clientWidth=305`；本地应用浏览器 `error/warning` 日志为空。
- 验证结束后仅停止显式隔离端口 5055 并清理临时目录；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v46 深链接安全落点证据（2026-08-12）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5056`，数据库位于
  `.cache/ui-v1018-runtime-20260812-5056/token_tracker-5056.sqlite3`；临时账户仅用于本地浏览器观察，写入三条临时记录，未读取真实数据库、
  Cookie、Key 或令牌。
- `static/ui-polish.css` 将既有锚点安全落点契约扩展到 `#auto-entry` 与 `#manual-entry`，继续使用
  `scroll-margin-top: clamp(84px, 7vw, 104px)`；不改模板 DOM、不新增滚动监听器，首屏 CTA 仍由原有 hash 导航驱动。
- 修复前真实点击 `开始自动采集` 并等待平滑滚动收束后，`#auto-entry` top≈`-15.90px`，低于 sticky header bottom=`76px`；修复后桌面
  `1683×892` 收束为 `entryTop≈104.10px`、标题 top≈`162.92px`，`scrollWidth=1668 / clientWidth=1668`。
- 直接打开 `#manual-entry` 深链接后确认 `manualTop≈92.05px`；320×800 移动视口点击同一 CTA 后确认 `entryTop≈84.03px`、
  `headingTop≈150.99px`、`scrollWidth=305 / clientWidth=305`（页面运行时微动效层不引入横向布局扩张）；本地应用浏览器 `error/warning` 日志为空。
- 验证结束后仅停止显式隔离端口 5056 并清理临时目录；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v47 透明品牌栏基础态证据（2026-08-12）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5057`，数据库位于
  `.cache/ui-v1019-runtime-20260812-5057/token_tracker-5057.sqlite3`；仅用于本地登录页观察，未读取真实数据库、
  Cookie、Key 或令牌。
- `static/ui-polish.css` 将基础态 `.site-header` 的 `backdrop-filter` 与外部阴影收敛为 `none`，保留透明背景、1px
  内侧高光、底部细边线和品牌文字阴影；滚动态羽化渐变仍由 `scene-motion.css` 原有状态层承载，不新增脚本或监听器。
- 桌面 `1683×892` 首屏真实计算值为 `background-color=rgba(0,0,0,0)`、`backdrop-filter=none`、仅保留内侧高光，
  页面 `scrollWidth=1668 / clientWidth=1668`；滚动后 `is-scrolled`、透明背景、`backdrop-filter=none`、header top=`0`、
  bottom=`76` 均成立。
- 320×800 真实计算值为透明背景、`backdrop-filter=none`、`scrollWidth=305 / clientWidth=305`，header 宽约 `274.67px`、
  高 `76px`；桌面首屏、滚动态和移动首屏截图均确认背景图可连续透过 AI TOKEN 行。本地应用浏览器 `error/warning` 日志为空。
- 验证结束后仅停止显式隔离端口 5057 并清理临时目录；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v48 登录表面半透明证据（2026-08-12）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5058`，数据库位于
  `.cache/ui-v1020-runtime-20260812-5058/token_tracker-5058.sqlite3`；仅用于登录/注册页面观察，未读取真实数据库、
  Cookie、Key 或令牌。
- `static/ui-polish.css` 将共享 `.auth-card` 的背景梯度由 `rgba(.62/.72)` 收敛为 `rgba(.48/.62)`，顶部光晕由
  `.14` 收敛为 `.11`，背景模糊由 `blur(18px) saturate(1.18)` 收敛为 `blur(12px) saturate(1.12)`；高对比度和 forced-colors
  兜底规则保持原有不透明阅读面。
- 桌面 `1683×892` 真实计算值确认登录卡片 `backdrop-filter=blur(12px) saturate(1.12)`、透明渐变层生效，header 仍为
  `background=rgba(0,0,0,0)`、`backdrop-filter=none`，页面 `scrollWidth=1668 / clientWidth=1668`。
- 320×800、768×900、1024×900、1440×900 均确认卡片存在、布局稳定且 `scrollWidth=clientWidth`（分别为 `305`、`753`、
  `1009`、`1425`）；注册页复用样式后同样无溢出，本地应用浏览器 `error/warning` 日志为空。
- 验证结束后仅停止显式隔离端口 5058 并清理临时目录；受保护的 5000/5011 进程与监听状态未触碰。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、浏览器下载落盘、真实 Provider 成功和 Android 真机验收。

## v49 交互态指针收敛证据（2026-08-12）

- 在当前 checkout 启动显式隔离数据库实例 `127.0.0.1:5059`，数据库位于
  `.cache/ui-v1021-runtime-20260812-5059/token_tracker-5059.sqlite3`；仅用于登录页 pointer/surface 观察，未读取真实数据库、
  Cookie、Key 或令牌。
- `static/scene-motion.css` 将交互态 `.pointer-aura` 从 `320px/.82/scale(1.08)` 收敛为 `300px/.68/scale(1.04)`，将
  `.pointer-ring` 从 `54px/.98` 收敛为 `48px/.92`，同步降低 glow；基础态 `36px` 和 reduced-motion/fine-pointer 门控保持不变。
- 桌面运行时将鼠标移至登录卡片与主按钮后确认 body 出现 `pointer-ready pointer-interactive`，卡片 `--surface-x/y` 随指针更新，
  按钮磁吸变量有值；收束计算值为 aura `300px/.68`、ring `48px`，离开交互目标后回到 aura `280px`、ring `36px`。
- 登录页真实浏览器 `error/warning` 日志为空；本轮不改变 DOM、ARIA、键盘焦点或移动/ reduced-motion 的 JS 退出条件。该证据不替代真实设备、
  `prefers-reduced-motion`/高对比度、下载落盘、真实 Provider 成功和 Android 真机验收。
- 验证结束后仅停止显式隔离端口 5059 并清理临时目录；受保护的 5000/5011 进程与监听状态未触碰。

## v50 滚动态品牌栏层次证据（2026-08-12）

- 在当前 checkout 启动显式隔离 Dashboard 实例 `127.0.0.1:5070`，数据库位于
  `.cache/ui-v1023-runtime-20260812-5070/token_tracker-5070.sqlite3`；账号仅用于本地空态观察，未读取真实数据库、Cookie、Key 或令牌。
- `static/scene-motion.css` 保留首屏 `.site-header` 的 `background=rgba(0,0,0,0)` 与 `backdrop-filter=none`；仅在 `is-scrolled` 状态增加
  `rgba(8,9,12,.16)` 基底、三段渐隐遮罩和 `blur(10px) saturate(1.08)`，使 AI TOKEN 行仍能透景但不再被下方统计文案穿透干扰。
- 桌面 `1683×892` 首屏截图确认品牌栏保持透明；点击“查看分析”后真实 `scrollY≈265`，header 进入 `is-scrolled`，计算样式为上述半透明渐隐层，分析空态、自动采集卡片和背景图层次保持稳定。
- 该实例真实计算值为 `scrollWidth=1668 / clientWidth=1668`，无横向溢出；首次 `Tab` 聚焦 skip-link 时为 `2px solid rgb(217,255,120)`、`outline-offset=4px`；本地应用浏览器 `error/warning` 日志为空。
- 本轮未伪造 320/768/1024/1440 视口证据；移动/高对比度/forced-colors/reduced-motion 继续引用 v47-v49 的已记录边界，待真实设备和批准浏览器矩阵复核。验证结束后仅清理显式隔离端口 5060/5070 及对应 `.cache/ui-v1022-runtime-20260812-5060`、`.cache/ui-v1023-runtime-20260812-5070`，受保护的 5000/5011 进程与监听状态未触碰。

## v51 深链接 reveal 收束证据（2026-08-12）

- 在当前 checkout 启动显式隔离 Dashboard 实例 `127.0.0.1:5071`，数据库位于
  `.cache/ui-v1024-runtime-20260812-5071/token_tracker-5071.sqlite3`；账号仅用于空态与导航观察，未读取真实数据库、Cookie、Key 或令牌。
- 修复前点击 Activity 的真实首帧为 `class=is-visible / opacity=.34 / translateY(12px) / transition-delay=.35s / duration=.82s`；这说明原生平滑滚动和文档顺序 reveal 同时占用进入动画，目标内容会在落点后继续等待。
- `static/scene-motion.css` 新增 `html.motion-ready [data-reveal]:target` 规则：只对当前 hash 目标清除 inline stagger delay，并将收束时间降为 `.56s`；普通滚动进入的 section 仍保留原有顺序 reveal。
- 修复后真实 Dashboard 深链接确认：Activity 稳定态为 `opacity=1 / transform=none / transition-delay=0s / duration=.56s`；Analysis 稳定态同样为 `top=92px / opacity=1 / transform=none / delay=0s`，sticky header 不遮挡标题。
- 桌面 `1683×892` 的 Dashboard 真实计算值为 `scrollWidth=1668 / clientWidth=1668`，无横向溢出；首次 `Tab` 仍聚焦 skip-link，焦点为 `2px solid rgb(217,255,120)`、`outline-offset=4px`；本地应用浏览器 `error/warning` 日志为空。
- 本轮未伪造 320/768/1024/1440 视口证据；移动/高对比度/forced-colors/reduced-motion 继续引用既有章节，待真实设备和批准浏览器矩阵复核。验证结束后仅清理显式隔离端口 5071 与对应 `.cache/ui-v1024-runtime-20260812-5071`，受保护的 5000/5011 进程与监听状态未触碰。

## v52 移动端首屏 CTA 证据（2026-08-12）

- 在当前 checkout 启动显式隔离 Dashboard 实例 `127.0.0.1:5072`，数据库位于
  `.cache/ui-v1025-runtime-20260812-5072/token_tracker-5072.sqlite3`；账号仅用于移动布局观察，未读取真实数据库、Cookie、Key 或令牌。
- 320×800 修复前真实计算值为 `hero-actions y=728 / height=106`，主 CTA `149px` 在第一行，次 CTA `123px` 落在 `y=786`，首屏底部仅可见约 14px；这是移动首屏入口被折行遮挡的直接证据。
- `static/style.css` 的 `max-width:620px` 规则将 `.hero-actions` 改为 `repeat(2, minmax(0, 1fr))`，按钮使用 `width:100%`、`min-width:0` 和紧凑间距；修复后 320×800 的两按钮均为 `134×48px`，坐标分别为 `x=15` 与 `x=157`、同为 `y=728`。
- 320px 真实值为 `scrollWidth=305 / clientWidth=305`；768×900 保持桌面式 `display=flex`，按钮宽度 `149/123px` 且 `scrollWidth=753 / clientWidth=753`；恢复默认 `1683×892` 后 CTA 仍为 `display=flex`、宽 `660px`，页面 `scrollWidth=1668 / clientWidth=1668`。
- 320px 修复后截图确认两个入口同时可见；键盘首个 `Tab` 仍聚焦 skip-link，焦点为 `2px solid rgb(217,255,120)`、`outline-offset=4px`；320/768/默认桌面本地应用浏览器 `error/warning` 日志均为空。
- 本轮未伪造 1024/1440 视口、真实设备、reduced-motion、forced-colors 或高对比度证据；viewport override 已在验证结束前 reset。仅清理显式隔离端口 5072 与对应 `.cache/ui-v1025-runtime-20260812-5072`，受保护的 5000/5011 进程与监听状态未触碰。

## v53 AI TOKEN 品牌栏透明层证据（2026-08-12）

- 复用当前 checkout 的显式隔离 Dashboard 实例 `127.0.0.1:5072` 与
  `.cache/ui-v1025-runtime-20260812-5072/token_tracker-5072.sqlite3`；仅观察已登录的合成空态页面，未读取真实 Cookie、Key、令牌或数据库。
- `static/scene-motion.css` 将 `.site-header` 与 `.site-header.is-scrolled` 的背景、背景图和 `backdrop-filter` 统一清零；移除滚动态原有深色渐隐层与 `blur(10px)`，保留边线、顶部细高光与既有滚动进度线。
- 默认桌面首屏真实 computed style：`scrollY=0`、`background=rgba(0,0,0,0)`、`backgroundImage=none`、`backdropFilter=none`。
  CUA 滚动至真实 `scrollY=1430` 后，header 为 `class=site-header is-scrolled`，仍为 `background=rgba(0,0,0,0)`、`backgroundImage=none`、`backdropFilter=none`；截图确认场景可透过品牌栏，底下卡片不会再被额外黑色渐隐层覆盖。
- 变更仅涉及装饰层 CSS，不改变 DOM、导航逻辑、键盘焦点、reduced-motion、forced-colors 或触摸降级；本地应用浏览器 `error/warning` 日志为空，`git diff --check` 通过。
- 本轮未伪造 320/768/1024/1440 视口、真实设备、辅助偏好和 Provider 联调证据；验证结束后仅停止显式隔离端口 5072 并清理对应临时目录，受保护的 5000/5011 进程与监听状态未触碰。

## v54 平板首屏主操作可见性证据（2026-08-12）

- 在当前 checkout 启动显式隔离 Dashboard 实例 `127.0.0.1:5073`，数据库位于
  `windows/.cache/ui-v1026-runtime-20260812-5073/token_tracker-5073.sqlite3`；仅使用合成账号观察布局，未读取真实 Cookie、Key、令牌或数据库。
- 修复前真实 768×900 计算值为 `.hero-orbit=480×480`、`.hero-actions y=1018.7`，主 CTA 完全落在首屏之外；900px 边界也沿用同一垂直超出风险。
- 新增 `static/responsive-tuning.css`，仅作用于 `min-width:621px` 到 `max-width:900px`：缩小 orbit 到 `min(46vw,360px)`，压缩 hero padding/gap 与 lede 留白；手机两列 CTA 和桌面双列 hero 不受该规则影响。
- 修复后真实结果：320×800 CTA 为 `y=727.9–775.9`、两列等宽 `133.6px`；768×900 CTA 为 `y=802–850`；900×900 CTA 为 `y=836–884`；1024×900 为 `y=639.1–687.1`；1440×900 为 `y=682.1–730.1`。五档均为 `scrollWidth=clientWidth`，无横向溢出。
- 768px 截图确认轨道仍保持视觉锚点、标题/文案/双 CTA 连续进入；900px 截图确认 CTA 完整可见且未改变按钮文字与焦点语义。首次 `Tab` 仍聚焦 skip-link，焦点为 `2px solid rgb(217,255,120)`、`outline-offset=4px`；本地应用浏览器 `error/warning` 日志为空。
- 本轮未伪造真实设备、reduced-motion、forced-colors、高对比度或 Provider 联调证据；验证结束后仅停止显式隔离端口 5073 并清理对应临时目录，受保护的 5000/5011 进程与监听状态未触碰。

## v55 顶部玻璃品牌栏与认证首屏证据（2026-08-12）

- 在当前 checkout 启动显式隔离认证实例 `127.0.0.1:5074`，数据库位于
  `.cache/ui-v1027-runtime-20260812-5074/token_tracker-5074.sqlite3`；仅使用认证空态页面观察布局，未读取真实 Cookie、Key、令牌或数据库。
- `static/scene-motion.css` 将 `.site-header` 改为低 alpha 渐变玻璃膜：首屏为 `linear-gradient(rgba(8,9,12,.14), rgba(8,9,12,.035))` + `blur(14px) saturate(1.12)`，滚动态提升为 `.20/.07` + `blur(18px) saturate(1.14)`；背景仍透景，未引入实心黑色面板。
- 真实浏览器 computed style 确认初始和滚动态均保留渐变背景图、透明背景色与预期 blur；滚动后 header 进入 `site-header is-scrolled`，仅增强边界层次和阴影。
- 390×844 登录按钮真实坐标约为 `y=497–545`、注册按钮约为 `y=506–554`，两者均在首屏内；手机 `.auth-card` 的 computed `order=-1`，介绍区在表单下方继续可访问。
- 768×900 登录/注册按钮分别落在约 `y=511–559` 与 `y=514–562`；1024×900 为约 `y=598–646` 与 `y=599–647`；1440×900 为约 `y=591–639` 与 `y=596–644`，六个认证状态均可见且无横向溢出。
- 390/768/1024/1440 四档 `scrollWidth=clientWidth`，页面控制台 `error/warning` 日志为空；手机和桌面截图确认表单优先层级与顶部透景层均实际生效。
- 源码保留 `forced-colors: active` 的系统配色复位与 `prefers-reduced-motion` 的顶栏过渡关闭；本轮未伪造真实设备、reduced-motion、forced-colors、高对比度、Provider 联调或真实账号证据。验证结束前应 reset viewport override，并仅清理端口 5074 及其显式隔离临时目录，受保护的 5000/5011 进程不得触碰。

## v56 移动 Dashboard 首屏节奏证据（2026-08-12）

- 在当前 checkout 启动显式隔离 Dashboard 实例 `127.0.0.1:5075`，数据库位于
  `.cache/ui-v1028-runtime-20260812-5075/token_tracker-5075.sqlite3`；仅使用合成账号观察空态页面，未读取真实 Cookie、Key、令牌或数据库。
- 修复前真实 390×844 主 CTA 坐标为 `y=787.9–835.9`，距离首屏底部仅约 8px；`.hero-orbit` 为 `335.4px`，移动首屏视觉锚点挤压了文案和操作区。
- `static/responsive-tuning.css` 的 `max-width:620px` 规则将 `.hero-orbit` 收敛为 `min(80vw,340px)`，并将 hero copy/lede 留白调整为 `margin-top=4px`、`22px/20px`，仅作用于移动 Dashboard。
- 修复后真实结果：320×800 CTA 为 `y=692.5–740.5`、轨道 `256px`；390×844 CTA 为 `y=747.5–795.5`、轨道 `312px`；两档均恢复约 48px 底部呼吸空间。
- 768×900 CTA 保持 `y=802.0–850.0`，1024×900 保持 `y=639.1–687.1`，1440×900 保持 `y=682.1–730.1`；五档均为 `scrollWidth=clientWidth`，无横向溢出。
- 390px 截图确认轨道仍是视觉锚点，标题、说明和两个 CTA 形成连续扫描层；源码已有 reduced-motion 全局降级，本轮未新增动画或交互语义，页面 `error/warning` 日志为空。
- 本轮未伪造真实设备、reduced-motion、forced-colors、高对比度、Provider 联调或真实账号证据；验证结束前应 reset viewport override，并仅清理端口 5075 及其显式隔离临时目录，受保护的 5000/5011 进程不得触碰。

## v57 Dashboard 信号层可读性证据（2026-08-12）

- 在当前 checkout 启动显式隔离 Dashboard 实例 `127.0.0.1:5076`，数据库位于
  `.cache/ui-v1029-runtime-20260812-5076/token_tracker-5076.sqlite3`；仅使用合成账号观察空态页面，未读取真实 Cookie、Key、令牌或数据库。
- 修复前 `#dashboard-status` 在动态插画上为透明背景、无边界、无文字阴影；`.hero-core` 仅有淡径向背景，核心文案没有阴影，状态与总量信标在复杂画面上缺少稳定阅读底。
- `static/scene-motion.css` 为状态文案加入低 alpha 胶囊、边界、`blur(8px)`、信号点和文字阴影；为 `.hero-core` 加入低 alpha 第二层与 `blur(8px)`，保留背景透景；错误状态和 forced-colors 均有独立复位。
- 320×800 真实值：状态胶囊为 `x=204.0,y=124.0,w=85.7,h=24.2`，保持单行；CTA 为 `y=698.6–746.6`。390×844 状态为 `107.3×29.5`，CTA 为 `y=758.8–806.8`。
- 768×900、1024×900、1440×900 的 CTA 分别为 `y=813.3–861.3`、`644.8–692.8`、`687.8–735.8`；状态胶囊与总量信标均在首屏可见，五档 `scrollWidth=clientWidth`。
- 五档真实截图确认状态胶囊与总量信标在角色/代码背景上仍然透景但更易扫描；页面 `error/warning` 日志为空。本轮未新增动画，既有 `prefers-reduced-motion` 全局降级继续生效。
- 本轮未伪造真实设备、reduced-motion、forced-colors、高对比度、Provider 联调或真实账号证据；验证结束前应 reset viewport override，并仅清理端口 5076 及其显式隔离临时目录，受保护的 5000/5011 进程不得触碰。

## v58 自动采集连接徽标证据（2026-08-12）

- 在当前 checkout 启动显式隔离 Dashboard 实例 `127.0.0.1:5078`，复用隔离数据库
  `.cache/ui-v1030-runtime-20260812-5077/token_tracker-5077.sqlite3`；使用新端口绕过旧静态 CSS 缓存，仅观察合成账号页面，未读取真实 Cookie、Key、令牌或数据库。
- 修复前 `.connection-badge` 的真实 computed style 为 `46×16.5px`，透明背景、无边界、无阴影、无 blur；“未连接”在自动采集卡片右上角缺少明确状态表面。
- `static/scene-motion.css` 为徽标增加 `63.3×27.8px` 的低 alpha 胶囊、边界、`blur(8px)`、内侧高光和状态色；`.is-ready`、`.is-error` 与 `forced-colors: active` 均有独立边界和颜色复位。
- 390×844 真实徽标坐标为 `x=277.7–341.0`，自动采集卡片宽 `344.7px`，未侵入页面右边界；768×900 为 `x=638.7–702.0`，1024×900 为 `x=539.4–602.7`，1440×900 为 `x=802.0–865.3`。
- 390/768/1024/1440 内容区均为 `scrollWidth=clientWidth`；无缓存截图确认徽标与自动采集卡片、右侧 HOW IT WORKS 面板使用同一透景玻璃语言，页面 `error/warning` 日志为空。
- 本轮未伪造真实设备、reduced-motion、forced-colors、高对比度或 Provider 联调证据；验证结束前应 reset viewport override，并仅清理端口 5077/5078 及其显式隔离临时目录，受保护的 5000/5011 进程不得触碰。

## v59 活动轨迹空态响应式证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5081`，复用隔离数据库
  `.cache/ui-v1031-runtime-20260812-5079/token_tracker-5079.sqlite3`；仅观察合成空态页面，未读取真实 Cookie、Key、令牌或用户数据库。
- 修复前真实 390px 空态活动表格为 `540px`，卡片表格容器可用宽度约 `307px`，`overflow-x=auto` 且 `scrollWidth=540`；1024px 双列布局中卡片约 `427px`，同样会继承 `540px` 最小宽度。这是空提示语被截断并出现横向滚动条的直接根因。
- `static/modules/activity.js` 让活动历史卡片随事件集合切换 `is-empty` 状态，模板提供首屏加载空态；`static/responsive-tuning.css` 仅对空态取消最小宽度、隐藏无意义表头并允许提示语换行，非空事件表格仍保留原有数据网格与横向浏览能力。
- 修复后真实值：390px 的空态表格为 `307.3px / min-width=0`，滚动容器 `clientWidth=307 / scrollWidth=307 / overflow-x=visible`；768px 为 `657.3/657`；1024px 为 `375.9/376`；1440px 为 `548.5/549`。四档页面 `scrollWidth=clientWidth`，空态提示语完整显示。
- 390×844 截图确认“当前还没有工作事件。完成一次 AI 辅助工作后再回来看看。”完整落在活动卡片内，底部隐私提示和表单按钮不被横向滚动条挤压；`uiTabV59.dev.logs()` 返回空数组。
- 本轮未改变表格数据字段、键盘顺序、ARIA live 状态或既有动画；未伪造真实非空事件、真实设备、reduced-motion、forced-colors、高对比度或 Provider 联调证据。验证结束前应 reset viewport override，并仅清理端口 5079/5080/5081 及对应隔离运行目录，受保护的 5000/5011 进程不得触碰。

## v60 分析图表空态密度证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5091`，使用隔离数据库
  `windows/.cache/ui-v1060-runtime-20260812-5090/token_tracker-5090.sqlite3` 与合成审计账号；未读取真实 Cookie、Key、令牌或用户数据库。
- 修复前真实空态图表在 390px 下分别约 `378.6px / 390.6px`，默认桌面分析区约 `416.7px`；大部分垂直空间是没有数据的黑色画布，连接区被推迟到更远的滚动位置。
- `static/modules/charts.js` 为 chart card 同步 `is-empty/is-unavailable` 状态；`static/responsive-tuning.css` 仅压缩这两类无数据状态的绘图区、标记和 CTA 间距，ready 状态不接收该规则，保持原 Chart.js 绘图区和交互。
- 修复后真实结果：320/390px 两张卡约 `348.6px / 345.6px`；768px 约 `348.4px / 345.4px`；1024px 同行高度约 `351.7px`；1440px 同行高度约 `366.7px`。五档页面均为 `scrollWidth=clientWidth`，空态标题、说明和“开始自动采集”入口完整显示。
- 320×800 截图确认两个空图表从大面积黑色留白变为紧凑的信号检查点，仍保留加号轨道、状态说明和明确 CTA；页面 `uiTabV60.dev.logs()` 返回空数组。
- 本轮未改变 Chart.js 数据格式、ready 状态、键盘顺序或 ARIA live 语义；未伪造真实非空图表、真实设备、reduced-motion、forced-colors、高对比度或 Provider 联调证据。隔离端口和视口已在验证结束前复位，受保护的 5000/5011 进程未触碰。

## v61 AI TOKEN 顶部透明优先证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5101`，复用隔离数据库
  `windows/.cache/ui-v1070-runtime-20260812-5100/token_tracker-5100.sqlite3`；仅使用合成账号观察布局，未读取真实 Cookie、Key、令牌或用户数据库。
- 修复前真实 header 首屏为 `linear-gradient(rgba(8,9,12,.14), rgba(8,9,12,.035))` + `blur(14px)`，滚动态为 `.20/.07` + `blur(18px)`；截图显示这层组合形成整条深色横带，切断了背景场景。
- `static/scene-motion.css` 现将首屏 header 设为 `background: transparent`、无 backdrop blur；滚动态仅使用约 `.045` alpha 的局部黑色 veil、`blur(4px) saturate(1.04)`、细边线和轻阴影，导航可读性与背景连续性同时保留。
- 真实首屏 computed style 在 320/390/768/1024/1440px 分别确认 `background=rgba(0,0,0,0)`、`backgroundImage=none`、`backdropFilter=none`，header 高度均为 `76px`；滚动态确认 `is-scrolled`、约 `.043` alpha 背景和 `blur(4px) saturate(1.04)`。
- 五档页面均保持 `scrollWidth=clientWidth`：320px 页面 `305/305`、390px `375/375`、768px `753/753`、1024px `1009/1009`、1440px `1425/1425`；未引入横向溢出。
- 桌面首屏/滚动态与 390px 手机首屏/滚动态截图确认 AI TOKEN 标识、导航和 Log out 仍可扫描，顶栏不再形成深色整条横带；`uiTabV61.dev.logs()` 返回空数组。
- 本轮未伪造真实设备、reduced-motion、forced-colors、高对比度或 Provider 联调证据；验证结束前已复位 viewport override，端口 5000/5011 未触碰，隔离运行目录按可恢复清理流程处理。

## v62 短高度桌面首屏周期切换器证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5110`，数据库位于
  `windows/.cache/ui-v1100-runtime-20260812-5110/token_tracker-5110.sqlite3`；仅使用合成账号观察布局，未读取真实 Cookie、Key、令牌或用户数据库。
- 修复前真实 1683×845 首屏中 `.hero-foot` 与 `.range-switcher` 的 bottom 均约为 `861.01px`，超出视口约 `16.01px`，周期切换器底部被截断。
- `static/responsive-tuning.css` 增加 `min-width:901px` + `max-height:860px` 的窄范围规则，将 `.hero-stage` 的下内边距从 `48px` 收敛为 `28px`；轨道尺寸、标题、CTA、移动和平板规则不变。
- 修复后真实 1683×845 的 `.hero-foot` bottom 为 `841.01px`、`.range-switcher` bottom 为 `841.01px`，在视口底部前保留约 `4px`；截图确认 Today/Week/Month/All time 完整可见。
- 真实响应式矩阵保持页面无横向溢出：320px `305/305`、390px `375/375`、768px `753/753`、1024px `1009/1009`、1440px `1425/1425`；1024/1440×900 周期切换器 bottom 均约 `874px`，完整位于首屏。
- 1683×845 与 1440×900 截图确认短桌面首屏操作层更完整，`uiTabV62.dev.logs()` 返回空数组；本轮未新增动画，既有 reduced-motion 与 forced-colors 规则保持。
- 本轮未伪造真实设备、reduced-motion、forced-colors、高对比度或 Provider 联调证据；验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v63 Dashboard 场景首帧稳定性证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5120`，数据库位于
  `windows/.cache/ui-v1200-runtime-20260812-5120/token_tracker-5120.sqlite3`；另以 `localhost:5120` 冷 origin 复核资源首帧，仅使用合成账号，未读取真实 Cookie、Key、令牌或用户数据库。
- 修复前真实 768×900 首次导航约 `120ms` 的截图中，正文与 TOTAL SIGNAL 已绘制，但场景插画尚未完成绘制，背景短暂呈近黑空画布；约 `1.8s` 后才恢复完整插画。
- `templates/base.html` 为 v14 场景加入 `rel=preload` + `fetchpriority=high`，并给实际背景 `<img>` 同步声明高优先级；`static/scene-motion.css` 为 `.story-backdrop` 增加 `#071321` 深蓝 fallback，解码间隙不再表现为纯黑遮罩。
- 冷 origin `localhost:5120` 的 768×900 首帧截图确认场景已经可见；同一时刻读取到 `story-backdrop-image.complete=true`、`naturalWidth=1672`、fallback computed color `rgb(7,19,33)`，页面控制台为空。
- 真实矩阵均确认图片已完成、无横向溢出：320px `305/305`、390px `375/375`、768px `753/753`、1024px `1009/1009`、1440px `1425/1425`；五档 `imageComplete=true`、`naturalWidth=1672`。
- 390×844 截图确认移动首屏仍保留角色、轨道、标题和双 CTA 的连续层级；本轮仅改变资源启动和加载 fallback，没有新增动画或业务状态，`uiTabV63Cold.dev.logs()` 返回空数组。
- 本轮未伪造真实设备、reduced-motion、forced-colors、高对比度或 Provider 联调证据；验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v64 透明顶栏文字锐化证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5130`，数据库位于
  `windows/.cache/ui-v1300-runtime-20260812-5130/token_tracker-5130.sqlite3`；仅使用合成账号观察首屏和 Activity 滚动态，未读取真实 Cookie、Key、令牌或用户数据库。
- 修复前真实 Activity 滚动态截图显示 `AI TOKEN / OBSERVATORY` 在明暗交错插画上呈宽泛发光，原 computed `text-shadow` 为 `0 1px 12px rgba(8,9,12,.52)`；小字号品牌锁定区边缘不够锐利。
- `static/ui-polish.css` 将共享顶栏文字阴影收敛为 `0 1px 2px rgba(8,9,12,.92), 0 0 8px rgba(8,9,12,.68)`：保留暗色 keyline 和短 halo，不增加背景填充、不改变透明顶栏策略。
- 真实 1440×900 首屏确认 header `background=rgba(0,0,0,0)`、`backdrop-filter=none`；稳定 Activity 滚动态确认 `site-header.is-scrolled`、约 `.043` alpha 背景、`blur(4px) saturate(1.04)` 和新 text-shadow，品牌文字 opacity 为 `1`。
- 真实响应式 computed matrix 确认 320/390/768/1024/1440 五档 brand opacity 均为 `1`，且页面宽度分别保持 `305/305`、`375/375`、`753/753`、`1009/1009`、`1425/1425`，无横向溢出。
- 1440×900 Activity 稳定滚动态与 390×844 首屏截图确认品牌、导航、Log out、状态胶囊和移动 CTA 仍可扫描；`uiTabV64.dev.logs()` 返回空数组，本轮未新增动画、DOM、API 或业务状态。
- 本轮未伪造真实设备、reduced-motion、forced-colors、高对比度或 Provider 联调证据；验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v65 锚点导航受控过渡证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5140`，数据库位于
  `windows/.cache/ui-v1400-runtime-20260812-5140/token_tracker-5140.sqlite3`；仅使用合成账号观察导航，不读取真实 Cookie、Key、令牌或用户数据库。
- 修复前真实 1440×900 从首屏点击 Activity 的原生 smooth scroll 在 `6537ms` 采样窗口内仍未达到稳定 section 落点，最终 section top 约 `51px`；动态空态布局还会让原始落点发生漂移。
- `static/modules/navigation.js` 现在接管当前页面的同文档锚点：按距离使用 `420–760ms` cubic easing，监听 wheel/touch/pointer/keyboard 取消未完成过渡，结束时重新计算 `scroll-margin-top` 后校准；hash 通过 `history.pushState` 保留，导航链接即时更新 `aria-current`，reduced-motion 直接跳转。
- 真实桌面验证：`#activity` 受控滚动后稳定 section top 约 `88.7px`、heading 位于 header 下方，hash 为 `#activity`，nav active 为 `Activity`，页面 `anchorScrollReady=true`。
- 真实移动验证：390×844 点击首屏“开始自动采集”后，`#auto-entry` 稳定 target top 约 `68px`、标题 top 约 `117px`，header bottom 为 `76px`，页面宽度 `375/375`；截图确认自动采集表单从正确落点开始阅读。
- 320/390/768/1024/1440 矩阵均确认 `anchorScrollReady=true`、`#activity/#auto-entry` 存在且页面无横向溢出，宽度分别为 `305/305`、`375/375`、`753/753`、`1009/1009`、`1425/1425`；`uiTabV65.dev.logs()` 返回空数组。
- 本轮未伪造真实 reduced-motion、forced-colors、真实设备或 Provider 联调证据；代码保留 reduced-motion 直达分支，验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v66 滚动状态透明顶栏证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5141`，数据库位于
  `windows/.cache/ui-v1400-runtime-20260812-5141/token_tracker-5141.sqlite3`；仅使用登录页合成状态观察顶栏，不读取真实 Cookie、Key、令牌或用户数据库。
- 修复前滚动态规则在 `.site-header.is-scrolled` 上仍设置 `rgba(8,9,12,.045)` 与 `blur(4px) saturate(1.04)`，会让本应透景的 AI TOKEN 行产生黑玻璃感；`static/scene-motion.css` 现在将滚动态与静止态统一为 `background=transparent`、`backdrop-filter=none`，保留发丝内阴影、底线和进度线。
- 真实 390×844 滚动后 `scrollY≈506`，header class 为 `site-header is-scrolled`，computed `background=rgba(0,0,0,0)`、`backdrop-filter=none`；截图确认品牌行直接叠在角色/场景上，没有横向黑色面板。
- 真实 1440×900 滚动后 `scrollY≈191.3`，同样得到 `background=rgba(0,0,0,0)`、`backdrop-filter=none`、仅保留 `rgba(255,255,255,.04) 0 1px 0 inset`；截图确认顶部 AI TOKEN、导航与滚动进度线可扫描。
- 320/390/768/1024/1440 矩阵均确认 header `background=rgba(0,0,0,0)`、`backdrop-filter=none`，页面宽度分别为 `305/320`、`375/390`、`753/768`、`1009/1024`、`1425/1440`，无横向溢出；`tabV66.dev.logs()` 返回空数组。
- 本轮浏览器工具自身曾出现外部遥测队列告警，但不属于本地页面日志；未将其计入应用缺陷。真实 reduced-motion、forced-colors、真实设备、Provider 联调和正式部署仍未宣称通过；验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v67 基础顶栏透明策略证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5142`，数据库位于
  `windows/.cache/ui-v1400-runtime-20260812-5142/token_tracker-5142.sqlite3`；仅使用登录/注册页合成状态观察渲染，不读取真实 Cookie、Key、令牌或用户数据库。
- 修复前 `static/style.css` 的基础 `.site-header` 先声明 `rgba(15,16,20,.84)` 与 `blur(20px)`，透明策略依赖后加载的 `ui-polish.css`/`scene-motion.css` 覆盖；现在基础规则直接声明 `background: transparent`、`-webkit-backdrop-filter: none`、`backdrop-filter: none`，后续层仅负责滚动态边界与主题例外。
- 真实 390×844 登录页首屏 computed 为 `background=rgba(0,0,0,0)`、`backdrop-filter=none`，截图确认 AI TOKEN 行直接透过背景场景，页面无横向溢出。
- 真实 1440×900 首屏 computed 为透明/无 blur；滚动约 `191.3px` 后 `site-header.is-scrolled` 仍为透明/无 blur，仅保留 `rgba(255,255,255,.04) 0 1px 0 inset`，截图确认品牌栏和登录卡片层级稳定。
- 320/390/768/1024/1440 矩阵均确认品牌存在、header 透明、无横向溢出，页面宽度分别为 `305/320`、`375/390`、`753/768`、`1009/1024`、`1425/1440`；`tabV67.dev.logs()` 返回空数组。
- 本轮未伪造真实 reduced-motion、forced-colors、真实设备或 Provider 联调证据；浏览器工具自身的外部网络遥测告警不计入本地应用日志。验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v68 认证焦点层级证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5143`，数据库位于
  `windows/.cache/ui-v1400-runtime-20260812-5143/token_tracker-5143.sqlite3`；仅使用登录页合成状态观察焦点，不读取真实 Cookie、Key、令牌或用户数据库。
- 修复前桌面 `auth.js` 的自动聚焦会直接命中 `input:focus-visible`，首屏用户名字段立即呈现完整荧光 halo；本轮以 `data-auth-autofocus` 标记自动聚焦窗口，首个 pointer/keyboard 事件后删除标记，独立 `static/auth-focus.css` 只对该短窗口使用 quiet edge。
- 真实桌面首屏截图确认自动聚焦的用户名字段不再显示大面积 halo；读取到 `active=username`、`marker=true`，用户操作前视觉保持安静。随后发送一次 Tab，marker 变为缺省，密码字段 `focus-visible=true`，computed border 为 `rgb(217,255,120)`、background 为 `rgba(217,255,120,.08)`、box-shadow 为 `3px` 键盘 halo；截图确认键盘反馈仍清晰。
- 真实 390×844 移动截图确认登录卡片、品牌栏和标题层级保持，`active=null`、无自动聚焦 marker、页面宽度 `375/390`；移动端不抢占软键盘入口。
- 320/390/768/1024/1440 矩阵均确认认证卡片与品牌存在、页面无横向溢出，宽度分别为 `305/320`、`375/390`、`753/768`、`1009/1024`、`1425/1440`；`tabV68.dev.logs()` 返回空数组。
- 本轮未伪造真实 reduced-motion、forced-colors、真实设备或 Provider 联调证据；浏览器工具自身外部网络遥测告警不计入本地页面日志。验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v69 顶部品牌栏完全透景与窄视口焦点边界证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离认证实例 `127.0.0.1:5146`，数据库位于
  `windows/.cache/ui-v1400-runtime-20260812-5146/token_tracker-5146.sqlite3`；仅使用登录页合成状态观察渲染，未读取真实 Cookie、Key、令牌或用户数据库。
- `static/ui-polish.css` 与 `static/scene-motion.css` 现在共同声明 `.site-header` / `.site-header.is-scrolled` 的 `background=transparent`、`background-image=none`、`border-bottom-color=transparent`、`box-shadow=none`；只保留文字阴影与滚动进度线，避免边线和内侧高光被看成黑色横带。
- 真实 1440×900 首屏确认上述透明计算值、`backdrop-filter=none` 与页面宽度 `1425/1425`；PageDown 后 `class=site-header is-scrolled`，仍保持透明/无阴影/无边线，进度线宽度约 `1320px`，截图确认场景直接穿过 AI TOKEN 行。
- 320/390/768/1024/1440 矩阵均确认品牌存在、header 背景/边线/阴影透明且页面无横向溢出，页面宽度分别为 `305/305`、`375/375`、`753/753`、`1009/1009`、`1425/1425`；移动窄视口 `active=null`、无 `data-auth-autofocus`，桌面自动聚焦仍保留 quiet edge。
- 桌面首屏读取到用户名字段 `background=rgba(217,255,120,.05)`、1px quiet edge；Tab 后密码字段恢复 `:focus-visible` 的 3px keyboard halo。`auth-focus.css` 已独立加载，`tabV68.dev.logs()` 返回空数组。
- 本轮未伪造真实 reduced-motion、forced-colors、真实设备或 Provider 联调证据；浏览器工具自身外部网络遥测告警不计入本地页面日志。验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v70 移动滚动态安全顶栏证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5148`，数据库位于
  `windows/.cache/ui-v1500-runtime-20260812-5148/token_tracker-5148.sqlite3`；仅使用合成账号观察移动层级，未读取真实 Cookie、Key、令牌或用户数据库。
- 基线 390×844 首屏仍确认 `.site-header` 为 `background=rgba(0,0,0,0)`、`backgroundImage=none`、`backdrop-filter=none`；移动滚动态新增短安全层：`linear-gradient(rgba(8,9,12,.54), rgba(8,9,12,.18) 78%, transparent)`、`blur(10px) saturate(1.08)` 和低幅阴影，仅遮住穿入顶栏的首屏 CTA。
- 真实 390px 滚动至 `scrollY≈700` 时确认 `site-header.is-scrolled`，首屏 CTA 几何约为 `top≈58.8px / bottom≈106.8px`；截图显示 AI TOKEN 与 Log out 保持清晰，CTA 不再直接穿过品牌操作层。1440px 滚动态仍为透明背景、无 blur、无阴影，桌面构图未改变。
- `responsive-tuning.css` 为移动安全层补齐 `forced-colors: active` 的 `Canvas/CanvasText` 复位；本轮未伪造真实 forced-colors、reduced-motion、设备或 Provider 联调证据。验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v71 桌面滚动态安全顶栏证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5150`，数据库位于
  `windows/.cache/ui-v1600-runtime-20260812-5150/token_tracker-5150.sqlite3`；仅使用合成账号观察布局，未读取真实 Cookie、Key、令牌或用户数据库。
- 1440×900 首屏保持 `.site-header` `background=rgba(0,0,0,0)`、`backgroundImage=none`、`backdrop-filter=none`、透明底边和无阴影；因此首屏背景插画仍然完整透过 AI TOKEN 行。
- 桌面滚动至 `scrollY≈900` 后进入 `site-header is-scrolled`，启用深蓝渐变 `rgba(7,19,33,.78/.56/.18)`、`blur(12px) saturate(1.08)`、`rgba(230,235,245,.14)` 底边和轻阴影；截图确认 INPUT TOKENS/ACTIVE MODELS 等指标不再穿入导航区，同时插画仍可见。
- 390×844 首屏为 `375/375` 且完全透明；滚动至 `scrollY=600` 后使用移动端较轻的 `.54/.18` 渐变、`blur(10px) saturate(1.08)` 与 `.12` 边界。320×800 首屏与滚动态分别为 `305/305`、`scrollY=520`，同样无横向溢出。
- `responsive-tuning.css` 将 `forced-colors: active` 复位提升为全断点规则，由 `Canvas/CanvasText` 接管滚动态安全层；本轮未伪造真实 forced-colors、reduced-motion、真实设备或 Provider 联调证据。
- `tabV1600.dev.logs()` 返回空数组；浏览器工具自身的外部遥测队列告警不计入本地页面日志。验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v72 透明顶栏文字对比度证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5160`，数据库位于
  `windows/.cache/ui-v1700-runtime-20260812-5160/token_tracker-5160.sqlite3`；仅使用合成账号观察布局，未读取真实 Cookie、Key、令牌或用户数据库。
- 修复前 1440×900 首屏的非活动导航与用户名 computed color 为 `rgb(174,180,193)`；`responsive-tuning.css` 现将透明顶栏的 quiet labels 提升为 `var(--ink-soft)`，实际读取为 `rgb(215,219,229)`，并使用 `rgba(8,9,12,.94/.76)` 的短 keyline/halo；活动项继续使用 `var(--ink)`。
- 1440×900 截图确认 AI TOKEN 行仍为 `background=rgba(0,0,0,0)`、`backgroundImage=none`、`backdrop-filter=none`，但 Analysis/Connect/Activity/History 与用户名在亮色插画区域仍可连续扫描；滚动态深蓝安全层与 12px blur 保持不变。
- 320/390/768/1024/1440 首屏矩阵均保持 `scrollWidth=clientWidth`，分别为 `305/305`、`375/375`、`753/753`、`1009/1009`、`1425/1425`；390px 滚动至 `scrollY=600` 后仍确认移动 veil、`blur(10px) saturate(1.08)` 与 `.12` 底边。
- Dashboard 可访问性快照确认 banner、主要导航、Log out、main、统计周期按钮、分析 region、表单控件与状态文本均保留可访问名称；`tabV1700.dev.logs()` 返回空数组。
- 本轮未伪造真实 reduced-motion、forced-colors、真实设备或 Provider 联调证据；浏览器工具自身外部遥测队列告警不计入本地页面日志。验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v73 透明指标栏文字可读性证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5170`，数据库位于
  `windows/.cache/ui-v1800-runtime-20260812-5170/token_tracker-5170.sqlite3`；仅使用合成账号观察布局，未读取真实 Cookie、Key、令牌或用户数据库。
- 修复前 `.signal-cell` 的背景与阴影均为无值/透明，第三格 `in selected range` 直接落在角色亮部；现仅为 `.signal-index`、`.stat-label`、`.stat-value` 和 `small` 增加 `rgba(8,9,12,.94/.7)` 局部字形 keyline，未增加面板背景。
- 390×844 滚动至 `scrollY=620` 的截图确认 INPUT/OUTPUT/ACTIVE MODELS 三格仍直接透景，`small` computed color 提升为 `rgb(215,219,229)`；1440×900 滚动态仍保持 `site-header.is-scrolled`、深蓝渐变与 `blur(12px) saturate(1.08)`。
- 390/1440 页面均保持 `scrollWidth=clientWidth`，分别为 `375/375` 与 `1425/1425`；`tabV1800.dev.logs()` 返回空数组。
- 本轮未伪造真实 reduced-motion、forced-colors、真实设备或 Provider 联调证据；浏览器工具自身外部遥测队列告警不计入本地页面日志。验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v74 reveal 过渡可读性证据（2026-08-12）

- 在当前 checkout 重新启动无缓存隔离 Dashboard 实例 `127.0.0.1:5170`，数据库位于
  `windows/.cache/ui-v1800-runtime-20260812-5170/token_tracker-5170.sqlite3`；仅使用运行时创建的合成账号观察布局，未读取真实 Cookie、Key、令牌或用户数据库。
- 基线复核确认 below-fold `data-reveal` 在 v73 过渡期间会进入 `opacity=.34 / translateY(12px)`；`responsive-tuning.css` 现仅在 `prefers-reduced-motion: no-preference` 下覆盖为 `opacity=.62 / translateY(8px) / .72s`，`.is-visible` 仍为 `opacity=1 / translateY(0)`，没有改变 DOM、数据或业务逻辑。
- 真实 1440×900 页面滚动到 `scrollY=1500` 后，在 120ms 观察窗口读取 `#analysis`、`#activity`、`#history` 等尚未落位节点，computed style 为 `opacity=.62`、矩阵位移约 `8px`；继续滚动后 `#history` 在 `scrollY=2970` 已落位为 `opacity=.999965`、位移约 `0px`，说明可读性地板与动效方向同时生效。
- 390×844 首屏读取 `scrollWidth/clientWidth=375/375`，header 为 `background=rgba(0,0,0,0)`、`backgroundImage=none`；320×780 为 `305/305` 且同样保持透明。768×860、1024×900、1440×900 分别为 `753/753`、`1009/1009`、`1425/1425`，均无横向溢出。
- Dashboard 无障碍快照长度为 `5965`，保留 `main`、`主要导航`、`TOKEN SIGNAL` 等关键语义；读取到 9 个标题、1 个 `main`，`tabV74.dev.logs({})` 返回空数组。浏览器默认媒体结果为 `no-preference=true / reduce=false`。
- reduced-motion 规则仍由 `ui-polish.css` 的独立 `@media (prefers-reduced-motion: reduce)` 复位；本轮未伪造 reduced-motion、forced-colors、真实设备或 Provider 联调证据。验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v75 AI TOKEN 顶栏透明契约证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5171`，数据库位于
  `windows/.cache/ui-v1900-runtime-20260812-5171/token_tracker-5171.sqlite3`；仅使用运行时创建的合成账号观察页面，未读取真实 Cookie、Key、令牌或用户数据库。
- 根因是 `scene-motion.css` 已声明滚动态透明，但后加载的 `responsive-tuning.css` 又写入深蓝渐变、`blur(12px)` 和阴影；v75 将后加载覆盖收敛为 `background=transparent`、`backgroundImage=none`、`backdrop-filter=none`、`box-shadow=none` 与透明底边线。普通模式不再出现顶栏黑色/深蓝面板；forced-colors 仍由系统 `Canvas` 规则接管。
- 真实 1440×900 登录页首屏与 Dashboard 首屏均读取透明顶栏；Dashboard 滚动至 `scrollY=900` 后 class 为 `site-header is-scrolled`，但 computed background 仍为 `rgba(0,0,0,0)`、背景图为 `none`、backdrop 为 `none`、底边线透明、阴影为 `none`。
- 390×844 首屏与 `scrollY=620` 滚动态均保持上述透明值，页面宽度为 `375/375`；320×780 为 `305/305`。768×860、1024×900、1440×900 滚动态分别为 `753/753`、`1009/1009`、`1425/1425`，均无横向溢出。
- Dashboard 无障碍快照长度为 `5965`，保留 banner、`主要导航`、main、`TOKEN SIGNAL` 等关键语义；读取到 9 个标题、1 个 main，`tabV75.dev.logs({})` 返回空数组。浏览器工具自身的 Statsig 队列告警未出现在本地页面日志中。
- 本轮未伪造真实 reduced-motion、forced-colors、真实设备或 Provider 联调证据；验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。

## v76 Dashboard 深蓝玻璃面板证据（2026-08-12）

- 在当前 checkout 启动无缓存隔离 Dashboard 实例 `127.0.0.1:5172`，数据库位于
  `windows/.cache/ui-v2000-runtime-20260812-5172/token_tracker-5172.sqlite3`；仅使用运行时创建的合成账号观察页面，未读取真实 Cookie、Key、令牌或用户数据库。
- 基线 computed style 显示空态图表为近不透明 `rgba(15,19,25,.984) → rgba(7,10,14,.996)`，自动采集/引导面板也在 `.96/.98` 附近；v76 将 `.chart-card`、`.form-card`、`.records-card`、`.guide-card` 和 `.manual-details` 统一到深蓝渐变 `.84/.92`，并启用 `blur(16px) saturate(1.08)`，保留文字与图表自身对比度。
- 真实 390×844、320×780、768×860、1024×900、1440×900 均读取 `backdrop-filter=blur(16px) saturate(1.08)`，页面宽度分别为 `375/375`、`305/305`、`753/753`、`1009/1009`、`1425/1425`，无横向溢出。
- 普通浏览器媒体结果为 `forced-colors: none`；玻璃规则被限定在 `@media (forced-colors: none)`，不会覆盖项目既有的系统 Canvas 复位。未伪造真实 forced-colors、reduced-motion、真实设备或 Provider 联调证据。
- Dashboard 无障碍快照长度为 `5965`，保留 banner、`主要导航`、main、`TOKEN SIGNAL` 等关键语义；读取到 9 个标题、1 个 main，`tabV76.dev.logs({})` 返回空数组。验证结束前应 reset viewport override，端口 5000/5011 不得触碰，隔离目录按可恢复清理流程处理。
