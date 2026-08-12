# v92 UI / 可访问性证据：自动采集操作行阅读边界

日期：2026-08-12

## 变更目标

v91 后的深度审计发现，自动采集表单的 `发送 · 自动记账` 按钮虽然已经退出 sticky 顶栏，但同一行的“Key 只保存在当前页面内存”说明仍会穿过 0–76px 阅读带。v92 将整个 `.auto-form-actions` 作为一个视觉单元处理，避免只隐藏按钮而留下残余说明。

## 源码边界

- `sticky-occlusion.js` 将 `.site-main .auto-form-actions` 纳入既有候选集合，复用 header 几何、滚动状态和 requestAnimationFrame 合并逻辑。
- `responsive-tuning.css` 继续只改变 `.is-under-sticky-header` 的 opacity、pointer-events 和过渡；操作行的布局高度、DOM、表单值和 Tab 顺序不变。
- 操作行内按钮仍保留自身候选类；父级 `:focus-within` 让按钮获得焦点时整行恢复，离开焦点后继续遵循交叠边界。
- forced-colors 下恢复可见/可交互，reduced-motion 下取消过渡；未触碰接入 API、认证和数据库逻辑。

## 真实浏览器检查

检查实例：隔离端口 `5188`，仅使用临时合成账号；未读取真实 Cookie、Key、令牌、localStorage 或用户数据库。

- 深度 `scrollY=2200`：`.auto-form-actions` 视口矩形约为 `top=46.4 / bottom=94.4`，获得 `is-under-sticky-header`；过渡完成后整行 `opacity=0`、`pointer-events=none`，提交按钮和隐私说明不再形成穿透残影。
- 键盘路径：通过真实 Tab 到达 `#proxy-submit` 后，浏览器将焦点保持在提交按钮；父行 `:focus-within=true`，父行和按钮均恢复 `opacity=1`、`pointer-events=auto`，未改变 Tab 顺序。
- 继续离开顶栏交叠带后，操作行恢复为无避让类、`opacity=1`、`pointer-events=auto`。
- 截图由真实浏览器捕获；滚动和键盘使用浏览器交互，没有通过页面脚本修改 DOM 或伪造滚动位置。

## 可访问性与运行日志

- 真实 Dashboard 检查到 42 个可见焦点控件，未发现缺少文本、ARIA、placeholder 或 label 的控件；隐藏 CSRF input 不计入统计。
- 标题层级保持 1 个 `h1` 与 8 个 `h2`。
- `scrollWidth=clientWidth=1668`，无横向溢出。
- `tabV92.dev.logs({})` 返回空数组；浏览器客户端偶发的 Statsig 超时/丢弃事件属于工具侧遥测，不是本地页面日志。
- 当前浏览器不具备 viewport override 能力，因此 320/390/768/1024/1440px、reduced-motion、forced-colors 和真实设备证据不在本轮重新宣称；既有版本证据继续有效。

## 交付结论

v92 让自动采集提交按钮及其隐私说明作为完整操作行离开透明 sticky 顶栏，解决深滚时的残余文字穿透，同时保留键盘焦点和正常阅读位置的恢复行为。
