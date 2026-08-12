# UI 可访问性与视觉证据 · v112

## 变更主题

Dashboard 深滚时，指标栏不再穿过透明的 AI TOKEN 顶栏形成第二层文字。复用 `sticky-occlusion.js` 的候选元素机制，将三个 `.signal-cell` 纳入既有遮挡边界；普通滚动交叠时淡出并停止指针命中，离开交叠区后恢复。未新增监听器、接口、依赖或数据字段。

隔离实例：`127.0.0.1:5213`，运行目录为 `windows/.cache/ui-v3900-runtime-20260812-5213`。受保护服务 `5000/5011` 未触碰。

## 浏览器证据

- 桌面 `1440×900` 首屏：`header` 为 `site-header`，三组指标为 `signal-cell`、`opacity=1`、`pointer-events=auto`；首屏画面保持透明透景，`overflowX=false`。
- 桌面稳定深滚：`scrollY=980`、`scrollTop=980`，`header` 为 `site-header is-scrolled`，顶栏底部为 `76px`；三组指标均为 `signal-cell is-under-sticky-header`，几何范围 `top=-79 / bottom=101`，计算样式 `opacity=0`、`pointer-events=none`，`overflowX=false`。
- 深滚结果只在指标单元实际穿过 `76px` 顶栏阅读带时生效；移动端指标单元按列堆叠，未进入顶栏交叠区时保持可见，未引入横向溢出。
- 页面加载完成（`document.readyState=complete`），应用页面日志为空；浏览器运行时的第三方 telemetry warning 不计入应用错误。

## 角色复核

- 前端：复用现有遮挡模块和 focus-within 恢复规则，保持普通态、键盘态、forced-colors 与 reduced-motion 边界。
- 后端：无后端、数据库、API 或认证改动。
- 架构师：变更限定在两个共享前端资源；未创建重复实现，文件行数保持低于 1000 行，回滚边界清晰。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
