# v88 UI / 可访问性证据：sticky 顶栏内容避让

日期：2026-08-12

## 变更目标

v87 已让 AI TOKEN 品牌区回到完整透景，但深度滚动到自动采集表单时，`发送 · 自动记账` 控件仍可能进入透明 sticky header 的 76px 阅读带，形成按钮视觉穿透。v88 只处理这条表现层边界：控件穿过顶栏时收起视觉权重，离开后恢复；键盘焦点中的控件始终保持可见。

## 源码边界

- 新增 `static/modules/sticky-occlusion.js`，集中计算 sticky header 与 `.site-main .button` / `summary` 的视口交叠状态。
- `static/app.js` 与 `static/admin.js` 在导航初始化后启用同一模块，保持 Dashboard/Admin 的表现策略一致；模块只读取几何并切换表现类，不触碰业务数据、表单值或 DOM 结构。
- `responsive-tuning.css` 对 `.is-under-sticky-header` 使用 `opacity: 0` 与 `pointer-events: none` 收束视觉层，但保留布局盒和 Tab 顺序；`:focus-within` 恢复 `opacity: 1` 与可交互指针，reduced-motion 下取消过渡；forced-colors 下始终恢复可见和可交互。
- hero actions 继续由 v87 的既有规则负责，不被新模块重复处理；forced-colors 仍使用既有系统配色边界。

## 真实浏览器检查

检查实例：隔离端口 `5184`，仅使用临时合成账号；未读取真实 Cookie、Key、令牌、localStorage 或用户数据库。

- 首屏 `scrollY≈0`：header class 为基础态，`background=rgba(0,0,0,0)`、`backdrop-filter=none`；`#proxy-submit` 无避让类、`opacity=1`、`pointer-events=auto`，hero actions 可见。
- 中段 `scrollY≈720`：header class 为 `site-header is-scrolled`，hero actions 为 `opacity=0`、`visibility=hidden`；`#proxy-submit` 尚未进入顶栏交叠带，保持 `opacity=1`、`pointer-events=auto`。
- 深度 `scrollY=2220`：header 仍为透明/无 blur；`#proxy-submit` 获得 `is-under-sticky-header`，视口矩形约为 `top=26.39px / bottom=74.39px`，计算样式为 `opacity=0`、`pointer-events=none`，因此透明 AI TOKEN 行不会被下方提交按钮穿透。
- 键盘焦点：通过真实 Tab 路径到达 `#proxy-submit` 后，浏览器将其滚动到可操作位置；`focusWithin=true`、避让类移除、`opacity=1`、`pointer-events=auto`，截图确认焦点环和按钮文案清晰可见。
- 截图由真实浏览器捕获；滚动使用浏览器滚轮/键盘交互，没有通过页面脚本伪造滚动位置或修改页面 DOM。

## 可访问性与运行日志

- 真实 Dashboard 检查到 42 个可见焦点控件，未发现缺少文本、ARIA、placeholder 或 label 的控件；隐藏 CSRF input 不计入可见焦点统计。
- 标题层级保持 1 个 `h1` 与 8 个 `h2`。
- `scrollWidth=clientWidth=1668`，无横向溢出。
- `tabV88.dev.logs({})` 返回空数组。浏览器客户端期间出现的 Statsig 网络超时属于工具侧遥测，不是页面日志，且不影响本地页面请求。
- 当前浏览器不具备 viewport override 能力，因此 320/390/768/1024/1440px、reduced-motion、forced-colors 和真实设备证据不在本轮重新宣称；既有版本证据继续有效。

## 交付结论

v88 补齐了深度滚动下透明 sticky header 的内容避让边界：非焦点控件不再以亮色穿过 AI TOKEN 行，焦点控件仍可见、可达、可操作，且没有新增 DOM、API、依赖或业务状态。
