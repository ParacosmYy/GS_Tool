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
- 历史 v6 CSS 曾切换到 `/static/assets/embedded-rust-engineer-bg-v6.png`；当前默认引用已由 v7 小节记录，v6 资产仍保留作为回滚版本。
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

- 当前默认场景已升级为 `embedded-rust-engineer-bg-v7.png`，Android 同步使用
  `embedded_rust_engineer_bg_v7.png`；两份 PNG 的 SHA-256 为
  `B48B415E8685B9CB8A32E000ABC148C124A5A7D171E441F03650CD6D30DA1926`。
- v7 右侧保留成年御姐气质的嵌入式 Rust/RL 工程师、银色 Pro 笔记本、桌面工作站、示波器和
  开发板；左侧维持低细节深色留白，便于正文和表单阅读。图片不含可读文案、logo 或真实连接状态。
- 已完成生成器输出与本地文件目视检查；本节不把 v6 截图冒充 v7 运行时截图。合法认证会话下的
  仪表盘、连接页、管理员页四档截图仍按 UI-3 门禁单独跟踪。

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

- 当前源码进程使用显式装饰性 `<img>` 承载 v7 主图，CSS 背景声明保留为回退；场景层使用
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

本证据只覆盖无需创建测试账户的登录运行时。仪表盘、连接、历史和管理员页面仍需在合法认证会话中分别完成截图、空态、错误态和键盘回归；在没有用户提供可用体验账号且项目禁止创建测试数据的情况下，不能把模板静态检查冒充为受保护页面运行时通过。UI-3 总闸门因此继续保持进行中。

## v10 场景可见度构图

- 当前默认场景仍使用 v7 同 SHA-256 资产；本轮只调整桌面遮罩和亮度，将右侧角色、Pro
  笔记本、工作站与示波器从低可见轮廓提升为可识别背景，同时保留左侧正文安全区。
- 普通模式、`prefers-contrast: more`、`forced-colors: active` 和 `prefers-reduced-motion`
  均保留独立降级路径；背景不参与语义、焦点或业务状态。
- 当前只完成临时源码服务的 1440px 登录页观察；不把它扩展为受保护页面或真实设备通过证据。

## v10 窄屏边界复核（当前记录）

- 当前源码在隔离端口 `5019` 和临时 SQLite 下重新渲染；未读取真实数据库、Cookie、Key 或令牌。
- `ui-audit-v10/login-v10-1440.png` 确认 v7 场景、正文层、认证卡片和焦点样式仍可读。
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
