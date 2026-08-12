# v90 UI / 可访问性证据：周期切换器透景层

日期：2026-08-12

## 变更目标

v89 后，首屏底部周期切换器仍以偏黑的高 alpha 外壳呈现，与透明 AI TOKEN 顶栏和深蓝玻璃内容面板的视觉语法不一致。v90 只调整周期容器的表现层，让场景可透过控制外壳，同时保留 active pill 的 lime 状态反馈。

## 源码边界

- `responsive-tuning.css` 在 `forced-colors: none` 下为 `.range-switcher` 提供 `rgba(18, 32, 48, .58)` 背景、`blur(12px) saturate(1.08)` 和轻微内侧高光。
- active pill 仍由现有 `range-switcher.js` 的 `--range-pill-width` / `--range-pill-x` 和现有 lime pseudo-element 负责；不改变 period API、请求、DOM 或状态模型。
- forced-colors 不进入新玻璃规则，系统配色继续拥有最终控制权。

## 真实浏览器检查

检查实例：隔离端口 `5186`，仅使用临时合成账号；未读取真实 Cookie、Key、令牌、localStorage 或用户数据库。

- 首屏 `scrollY=0`：周期容器计算样式为 `background=rgba(18, 32, 48, 0.58)`、`backdrop-filter=blur(12px) saturate(1.08)`；active pill pseudo-element 为 `rgb(217, 255, 120)`，AI TOKEN header 仍为透明/无 blur。
- 通过真实点击依次验证 Today、Week、Month、All time：四档均将 `aria-pressed` 同步为 `true`，状态文案依次更新为“已更新：今天 / 本周 / 本月 / 全部”，pill 几何随按钮宽度移动。
- 中段滚动态截图确认周期控件不改变页面横向布局；内容玻璃层、导航局部 lens 和顶栏透明契约保持原样。
- 截图由真实浏览器捕获；交互使用真实按钮点击和滚轮，没有通过页面脚本伪造状态或修改 DOM。

## 可访问性与运行日志

- 真实 Dashboard 检查到 42 个可见焦点控件，未发现缺少文本、ARIA、placeholder 或 label 的控件；隐藏 CSRF input 不计入统计。
- 标题层级保持 1 个 `h1` 与 8 个 `h2`。
- `scrollWidth=clientWidth=1668`，无横向溢出。
- `tabV90.dev.logs({})` 返回空数组；浏览器客户端偶发的 Statsig 超时属于工具侧遥测，不是本地页面日志。
- 当前浏览器不具备 viewport override 能力，因此 320/390/768/1024/1440px、reduced-motion、forced-colors 和真实设备证据不在本轮重新宣称；既有版本证据继续有效。

## 交付结论

v90 让周期切换器融入插画场景而不退化为黑色浮块，同时保留清晰的 active 状态、键盘语义和既有周期数据契约。
