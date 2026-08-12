# UI 可访问性与视觉证据 · v116

## 变更主题

将 Dashboard 的趋势图与模型占比空态收敛为“observatory signal bay”：低 alpha 深蓝透景面、细坐标网格、`NO SIGNAL / READY` 装饰标记、局部顶部 signal line 和更稳定的空态文案阅读层。周期切换器增加与 hero-foot signal line 对齐的局部 hairline。数据、Chart.js canvas、API、DOM 语义和业务状态均不改变。

隔离实例：`127.0.0.1:5217`，运行目录为 `windows/.cache/ui-v116-runtime-20260812-5217`。受保护服务 `5000/5011` 未触碰。

## 浏览器证据

- `1440×900` Dashboard 空态：两张图表均为 `data-chart-state=empty`；空态 status 文案与“开始自动采集”入口保留，新增观测网格和顶部 signal line，图表卡片使用 `blur(8px)` 的轻量透景层而非原有更重材质。
- 深滚至 `scrollY=820`：指标栏、两张空态图表、自动采集卡和说明卡均保持 `opacity=1`；空态 chart wrap 的 `NO SIGNAL / READY` 伪元素、网格边界和阅读层正常渲染，页面无横向溢出。
- `320×720`、`390×844`、`768×900`、`1024×900`、`1440×900`：均无横向溢出；图表在窄屏保持单列，标题层级为 `1 个 h1 / 8 个 h2`；空态 `role=status` 和自动采集链接均存在。
- 周期切换真实点击 `Week` 后：按钮状态为 `Today=false / Week=true / Month=false / All time=false`，`aria-pressed` 同步；周期 hairline 在 focus-within 时降低透明度，避免与键盘焦点竞争。
- 浏览器应用日志数量为 `0`；宿主 telemetry warning 不计入应用错误。

## 动效协议

- 信息：网格和 `NO SIGNAL / READY` 表达“数据面已就绪但尚无记录”；顶部 signal line 表达分析卡边界；不伪造业务数据。
- 触发：沿用既有空态 marker 呼吸和周期控件交互；新增视觉层只使用伪元素、opacity、background 和既有 transform，不增加监听器或布局测量。
- 降级：`prefers-reduced-motion` 关闭空态 marker 动画；`forced-colors` 使用 `Canvas/CanvasText`，移除 blur、阴影和渐变竞争。
- 设备：移动端图表单列、原生滚动和触摸操作保持；周期按钮继续使用真实 button、focus-visible 和 `aria-pressed`。

## 角色复核

- UI：空态由“普通卡片”变成连续的观测舱，减少材质断层，不增加无意义圆角、黑色横带或伪造数据。
- 前端：新增 `observatory-signal.css`，通过 `base.html` 稳定 stylesheet contract 接入；图表模块、range-switcher 模块和接口不变。
- 后端：无后端、数据库、接口或密钥处理改动。
- 架构师：职责限定在 `03-observatory` 视觉层，使用现有 DOM 状态类和稳定 CSS contract；文件行数低于 1000 行，依赖方向和回滚边界清晰。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
