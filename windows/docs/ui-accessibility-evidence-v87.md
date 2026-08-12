# v87 UI / 可访问性证据：品牌区透景与滚动态 CTA 收束

日期：2026-08-12

## 变更目标

v86 的整条 header blur 会把滚动内容中的亮色 CTA 扩散到左侧 `AI TOKEN` 品牌区，产生荧光雾团。v87 将模糊范围收窄到桌面/平板右侧导航和账户操作区，并在离开首屏后收起一次性的 hero actions；品牌区继续保持完全透景。

## 源码边界

- `static/responsive-tuning.css` 将 `.site-header.is-scrolled` 恢复为透明背景、无背景图、无底边线、无阴影和 `backdrop-filter: none`。
- `site-nav::before` 与 `.user-menu::before` 只在 `min-width: 621px` 的滚动态启用透明 `blur(10px) saturate(1.04)` 局部镜片；镜片 `pointer-events: none`，forced-colors 下隐藏。
- `.site-header.is-scrolled + .site-main .hero-actions:not(:focus-within)` 使用 opacity/visibility 收束，避免首屏 CTA 经过 sticky 品牌行；`:focus-within` 保留键盘用户正在操作的按钮，reduced-motion 下取消过渡。
- 不新增 DOM、API、依赖、认证逻辑、业务状态或数据流；普通首屏与返回顶部行为不变。

## 真实浏览器检查

检查实例：隔离端口 `5183`，仅使用临时合成账号；未读取真实 Cookie、Key、令牌、localStorage 或用户数据库。

- 首屏 `scrollY=0`：header class 为基础态，`background=rgba(0,0,0,0)`、`backdrop-filter=none`；hero actions 的 `opacity=1`、`visibility=visible`、`pointer-events=auto`，`Scroll to explore` 同样可见。
- 中段 `scrollY=720` 稳定后：header class 为 `site-header is-scrolled`，header 与 brand 均为透明/无 blur；导航和账户局部伪元素为透明背景 `blur(10px) saturate(1.04)`；hero actions 为 `opacity=0`、`visibility=hidden`、`pointer-events=none`、`translateY(-10px)`，cue 为 `opacity=0`。
- 中段 `#analysis h2` top 约 `421px`，sticky header bottom 为 `76px`，标题不进入顶栏阅读区；回到 `scrollY=0` 后 hero actions 与 cue 均恢复。
- 深度 `scrollY=2220`：header 仍为透明/无 blur，hero actions 已离开文档视口，导航局部镜片仍不形成实心横条。本轮没有改变自动采集表单在其自身区域的交互。
- 截图由真实浏览器捕获；本轮没有通过页面脚本伪造滚动位置或修改页面 DOM。

## 可访问性与运行日志

- 真实 Dashboard 检查到 43 个焦点控件，未发现缺少可见文本、ARIA、placeholder 或 label 的控件。
- 标题层级保持 1 个 `h1` 与 8 个 `h2`。
- `scrollWidth=clientWidth=1668`，无横向溢出。
- `tabV87.dev.logs({})` 返回空数组。
- 当前浏览器不具备 viewport override 能力，因此 320/390/768/1024/1440px、reduced-motion、forced-colors 和真实设备证据不在本轮重新宣称；既有版本证据继续有效。

## 交付结论

v87 让滚动态的 `AI TOKEN` 品牌区回到真正透景，同时把必要的导航可读性收敛到右侧局部镜片，并撤下已经完成任务的首屏 CTA。深度滚动下更广泛的内容穿过 sticky header 的自动避让属于后续独立切片，不在本轮扩大范围。
