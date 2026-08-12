# UI 可访问性与视觉证据 · v123

## 变更主题

本轮修复透明 AI TOKEN sticky 顶栏在旧个人启动器样式链下的遮挡兼容边界。当前受保护的 `5000` 服务只加载基础 `style.css` 与 `ui-polish.css`，但 sticky occlusion 脚本仍会给深滚内容加上 `is-under-sticky-header` / `has-sticky-occlusion` class；此前基础层没有对应规则，分析卡片标题会穿过透明品牌行。现在基础样式提供与完整响应式模块一致的视觉遮挡、焦点恢复、reduced-motion 和 forced-colors 契约。

用户服务：`127.0.0.1:5000`。验证期间未重启或修改受保护的 `5000/5011` 服务，也未改变数据库、认证、Provider Key 或业务数据。

## 浏览器证据

- 使用真实 Dashboard 页面完成 `320×720`、`320×844`、`390×844`、`768×900`、`1024×900`、`1440×900` 六档几何检查；所有档位没有正向横向溢出，`document.documentElement.scrollWidth - innerWidth` 为 `-15px`（滚动条占位），页面实际内容没有超出视口。
- 深滚到分析区时，分析卡片标题在 sticky 顶栏覆盖区获得 `opacity: 0`、`pointer-events: none`，卡片保留正常布局；卡片 mask 从顶部 `0px` 到品牌栏底部 `66.625px` 的范围生效，标题不再穿透透明 AI TOKEN 行。
- 页面顶栏计算样式保持 `background: rgba(0, 0, 0, 0)`、`backdrop-filter: none`、高度约 `76px`；本轮没有用不透明遮罩掩盖问题。
- `390×844` 移动截图中分析卡片标题、空态图表和“开始自动采集”入口保持可读；hero 高度约 `781px`，分析区从约 `y=1353` 开始，v122 的移动首屏节奏没有回归。
- 六档均保持 `1 个 h1 / 8 个 h2`。点击 `Week` 后 `aria-pressed=true`、状态文案为“已更新：本周”；随后使用键盘 Tab，焦点移动到 `Month`，焦点轮廓为 lime `solid`，可见且未被遮挡。
- 应用页面日志为空，未发现页面级 console error/warn。

## 工程与无障碍协议

- 只在基础 `style.css` 增加 sticky 视觉边界；不新增 DOM、依赖、事件监听器、网络请求、认证逻辑或数据状态。
- 焦点元素通过 `:focus-within` 恢复可见与指针命中；`prefers-reduced-motion` 移除过渡，`forced-colors` 取消 mask 并恢复系统可见性。
- 基础 `style.css` 共 `673` 行，仍未超过项目每文件 1000 行约束；专用 `responsive-tuning.css` 继续保留完整模板下的同契约实现。
- 修复兼容边界而非修改业务脚本：`sticky-occlusion.js` 的 class 和 CSS 自定义属性契约保持不变。

## 角色复核

- UI：深滚分析标题不再被透明品牌栏截断，页面恢复“内容穿过场景、但不穿过阅读带”的层级秩序。
- 前端：基础层补齐已有 sticky class 的视觉实现，完整样式链继续由响应式模块提供，不改变模板语义、周期按钮脚本或 API。
- 后端：无后端、数据库、认证、Provider、Key 生命周期或数据流改动；保护端口 `5000/5011` 未触碰。
- 架构师：变化归属 `01-shell / sticky chrome compatibility boundary`，复用现有 class、CSS 变量和 focus contract，无新依赖、无跨模块耦合，回滚边界为 v123 基础样式块。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
