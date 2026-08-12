# UI 可访问性与视觉证据 · v113

## 变更主题

将普通配色下的 AI TOKEN 顶栏从低 alpha 渐变收敛为真正透明窗口：`background: transparent`、`background-image: none`、`backdrop-filter: none`、`box-shadow: none`。文字 keyline 与独立滚动进度线仍负责可读性和状态反馈，顶栏不再在背景插画上形成浅色横带。

隔离实例：`127.0.0.1:5214`，运行目录为 `windows/.cache/ui-v3900-runtime-20260812-5214d`。生产模式预检失败现场已另行保留到 quarantine；5000/5011 受保护服务未触碰。

## 浏览器证据

- `1440×900` 首屏：顶栏计算样式为 `background-image=none`、`backdrop-filter=none`、`box-shadow=none`，滚动进度线仍由 `::after` 独立存在；`overflowX=false`。
- `1440×900`、`scrollY=980` 与 `scrollY=1400`：顶栏为 `site-header is-scrolled`，仍保持 `background-image=none`、`backdrop-filter=none`、`box-shadow=none`，未改变透明场景；`overflowX=false`。
- `320×720`、`390×844`、`768×900`、`1024×900`、`1440×900`：均无横向溢出；标题层级为 `1 个 h1 / 8 个 h2`；43 个交互元素中 42 个具备可读名称。
- 移动端指标栏保持按列布局，顶栏计算高度 `76px`，指标在未交叠时保持 `opacity=1`；登录页与 Dashboard 均完成真实截图检查。
- 应用页面日志数量为 `0`；浏览器运行时的第三方 telemetry warning 不计入应用错误。

## 角色复核

- UI：去除顶栏渐变与阴影，保留文字 keyline、滚动进度线和现有品牌排版；不增加装饰层。
- 前端：只修改共享 `responsive-tuning.css`，不改变模板、交互、API、认证或滚动逻辑。
- 后端：无后端、数据库、接口或密钥处理改动。
- 架构师：变更保持在共享视觉契约边界，文件行数低于 1000 行，回滚为单文件局部提交。

## 后续观察项

浏览器回归发现现有滚动进度线在本次隔离运行中计算宽度为 `0px`；该行为与本轮透明化无关，保留为下一轮独立 UI 迭代项，避免扩大本轮改动范围。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
