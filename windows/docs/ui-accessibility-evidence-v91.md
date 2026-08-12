# v91 UI / 可访问性证据：sticky 标题阅读边界

日期：2026-08-12

## 变更目标

v90 的深度滚动态截图显示，`scrollY=1118` 时分析区两个 `.card-heading` 的视口矩形约为 `top=-3 / bottom=68`，标题文本穿过 0–76px 的透明 sticky 顶栏，造成“用量趋势 / 模型占比”被截断。v91 复用现有 sticky 避让模块，让穿越阅读带的卡片标题暂时退出视觉层，离开后恢复。

## 源码边界

- `sticky-occlusion.js` 将静态候选扩展为 `.site-main .card-heading`，与既有按钮和 `summary` 使用同一视口交叠计算、rAF 合并和 `:focus-within` 保护。
- `responsive-tuning.css` 继续通过 `.is-under-sticky-header` 只改变 opacity、pointer-events 和过渡，不改变标题布局盒、DOM、文案、API 或数据。
- 标题仍保留在 DOM 与辅助技术阅读路径中；管理员标题内的关闭按钮若获得焦点，父级 `:focus-within` 会恢复视觉和交互。
- forced-colors 下控件恢复可见/可交互，reduced-motion 下取消过渡；hero actions 继续由 v87 专属规则处理。

## 真实浏览器检查

检查实例：隔离端口 `5187`，仅使用临时合成账号；未读取真实 Cookie、Key、令牌、localStorage 或用户数据库。

- 深度 `scrollY=1118`：header 为 `site-header is-scrolled`，仍为 `background=rgba(0,0,0,0)`、`backdrop-filter=none`；分析区两个标题均获得 `is-under-sticky-header`，过渡完成后 `opacity=0`、`pointer-events=none`，截图确认顶栏阅读带不再出现截断标题。
- 返回 `scrollY=898`：分析标题矩形约为 `top=217 / bottom=288`，避让类移除，`opacity=1`、`pointer-events=auto`，标题正常恢复。
- 截图由真实浏览器捕获；滚动使用浏览器滚轮交互，没有通过页面脚本修改 DOM 或伪造滚动位置。

## 可访问性与运行日志

- 真实 Dashboard 检查到 42 个可见焦点控件，未发现缺少文本、ARIA、placeholder 或 label 的控件；隐藏 CSRF input 不计入统计。
- 标题层级保持 1 个 `h1` 与 8 个 `h2`。
- `scrollWidth=clientWidth=1668`，无横向溢出。
- `tabV91.dev.logs({})` 返回空数组；浏览器客户端偶发的 Statsig 超时/丢弃事件属于工具侧遥测，不是本地页面日志。
- 当前浏览器不具备 viewport override 能力，因此 320/390/768/1024/1440px、reduced-motion、forced-colors 和真实设备证据不在本轮重新宣称；既有版本证据继续有效。

## 交付结论

v91 补齐了透明 sticky 顶栏的标题阅读边界：标题不再以截断状态穿过 AI TOKEN 行，返回正常阅读位置后完整恢复，且不改变页面结构或辅助语义。
