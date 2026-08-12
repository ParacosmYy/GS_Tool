# v86 UI / 可访问性证据：透明滚动态顶栏的局部阅读层

日期：2026-08-12

## 变更目标

v85 之后的真实滚动检查发现，sticky `AI TOKEN` 品牌行虽然没有实心背景，深度滚动时仍会与经过顶栏的图表标题、首屏 CTA 形成竞争。v86 只调整滚动态的视觉层：保持填充、背景图、边线和阴影透明，用无填充的背景模糊降低穿过顶栏的内容对品牌/导航的干扰。

## 源码边界

- `static/responsive-tuning.css` 的 `.site-header.is-scrolled` 现在使用 `background: transparent`、`background-image: none`、透明底边线、`box-shadow: none` 与 `blur(10px) saturate(1.04)`。
- `scene-motion.css` 保留基础透明策略；后加载的 `responsive-tuning.css` 是滚动态视觉契约的唯一实际覆盖层，避免重复来源。
- 不新增 DOM、API、依赖、认证逻辑、业务状态或数据流；forced-colors 规则仍将滚动态复位到系统 `Canvas`，首屏仍不启用 blur。

## 真实浏览器检查

检查实例：隔离端口 `5182`，仅使用临时合成账号；未读取真实 Cookie、Key、令牌、localStorage 或用户数据库。

- 首屏 `scrollY=0`：`site-header` class 为基础态，`background=rgba(0,0,0,0)`、`backdrop-filter=none`、透明底边线、无阴影。
- 中段 `scrollY=720` 稳定后：class 为 `site-header is-scrolled`，`background=rgba(0,0,0,0)`、`backdrop-filter=blur(10px) saturate(1.04)`、底边线透明、阴影为 `none`；`Scroll to explore` 的 `opacity=0`、`pointer-events=none`、`transform=translateY(-6px)`。
- 交界位置 `scrollY=900`：sticky header bottom 约 `76px`，用量趋势、模型占比、自动采集和工作说明标题均从 `245px`/`626px` 起，不进入顶栏阅读区。
- 深度滚动 `scrollY=1118`：顶栏仍保持透明与无阴影，品牌、导航和账户操作可扫描；稳定截图未出现此前的黑色实心横条。
- 顶部、中段和深度滚动截图均由真实浏览器捕获；本轮没有修改页面 DOM 或通过页面脚本伪造滚动位置。

## 可访问性与运行日志

- 页面焦点控件共发现 42 个；datetime-local 字段通过父级 `label` 关联“记录时间”，隐藏 input 不参与视觉焦点路径。
- 标题层级保持 1 个 `h1` 与 8 个 `h2`，没有新增交互节点或焦点 stop。
- `tabV86.dev.logs({})` 返回空数组。
- 未具备 viewport override 能力，因此 320/390/768/1024/1440px、reduced-motion、forced-colors 和真实设备证据不在本轮重新宣称；既有证据仍按各自版本保留。

## 交付结论

v86 将滚动态顶栏收敛为“透明填充 + 轻量背景模糊”的局部阅读层，解决黑色横条观感，同时保留首屏完整透景和系统配色降级。未改变业务行为；后续若真实设备仍出现标题落入 sticky header 的情况，应单独设计滚动锚点/自动隐藏导航切片，不在本 CSS 增量中扩大范围。
