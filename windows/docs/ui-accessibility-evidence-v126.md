# UI 可访问性与视觉证据 · v126

## 变更主题

本轮修复旧个人入口中 `AI TOKEN / OBSERVATORY` 元信息行的全宽底线。该底线来自基础布局层，会把背景插画从中间切成一条横向视觉断层；完整样式链虽然已有局部信号线，但 `5000` 旧入口只加载 `style.css + ui-polish.css`，因此需要在兼容层补齐同一契约。审计同时发现通用 `ui-polish.css` 达到 1019 行，本轮把 Admin 专属规则拆到 `admin-polish.css`，恢复单文件硬上限。

现在 `.hero-topline` 保持背景透明、取消全宽 `border-bottom`，只在左侧绘制 `220px` 的 lime/lavender 局部 signal trace。`forced-colors: active` 下恢复系统 `CanvasText` 全宽边界，避免可访问性模式丢失定位线。

## 浏览器证据

- 真实 `127.0.0.1:5000/dashboard?ui_round=126` 页面加载后截图确认：顶部品牌栏保持半透明玻璃，`AI TOKEN / OBSERVATORY` 行背景透明，横向底线不再贯穿插画，局部 signal trace 可见。
- 运行态计算样式确认：`.hero-topline` `background` 为透明、`border-bottom-color` 为透明；桌面容器下伪元素 `width=220px`、`height=1px`、`left=0`、`opacity=.82`。
- 真实 `390×844` viewport 刷新后确认：`scrollWidth=375`、视口宽 `390`，差值来自垂直滚动条，没有正向横向溢出；trace 宽度按容器收缩为约 `89.6px`，背景和透明边界保持一致。
- 真实桌面默认窗口与移动回归确认：品牌、状态、总量轨道、标题、CTA 与周期控制仍保持原有布局顺序；没有新增业务 DOM、脚本监听器或业务状态。
- `admin.html` 源码明确加载 `/static/admin-polish.css`，静态资源返回 `200`；未认证访问 `/admin` 被既有保护逻辑拦截，因此本轮不把未登录页面冒充为管理员视觉验收。

## 工程与无障碍协议

- 变化限定在 `ui-polish.css` 兼容视觉层、`admin-polish.css` Admin 视觉模块和 `admin.html` 样式链接；没有改动 API、认证、数据库、Provider Key 或记录数据。
- 局部 trace 使用伪元素，不改变文档几何，不影响 Tab 顺序、屏幕阅读器语义或点击区域。
- `forced-colors: active` 显式恢复 `.hero-topline` 全宽 `CanvasText` 边界与实色伪元素；普通模式保持场景透景。
- Git 跟踪文本文件行数门禁、Python 编译和 `git diff --check` 通过；`ui-polish.css` 为 `867` 行、`admin-polish.css` 为 `90` 行，所有文件均低于 1000 行，审计结果为 `14 pass / 1 pending / 0 fail`。

## 角色复核

- UI：消除插画横向切割线，顶部信息层与角色、设备和背景监控屏连续融合。
- 前端：复用现有 `.hero-topline` DOM 与 signal token；Admin 样式通过独立 stylesheet link 暴露，不新增组件、依赖或事件分支。
- 后端：无后端、数据库、接口、认证、CSRF、Provider 或 Key 生命周期改动。
- 架构师：变化限定在 `01-shell / observatory metadata compatibility` 与 `05-history / admin visual boundary`；通用样式和 Admin 样式职责分离，回滚边界为局部 CSS 与 Admin stylesheet link。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
