# UI 可访问性与视觉证据 · v120

## 变更主题

修复用户反馈的“最上面 AI TOKEN 一行像被黑底覆盖”问题。根因是正在运行的个人入口仍只加载旧版基础样式链：`.site-header` 虽然透明，但 `.story-backdrop` 没有尺寸、背景图和场景层，透明区域实际落到黑色 `body` 画布。修复在基础 `style.css` 增加兼容场景边界；当前完整样式链继续由 `scene-motion.css` 接管，不改变认证、导航、接口、数据库或数据流。

隔离复现服务：`127.0.0.1:5221`，仅用于本地视觉观察；用户保护服务 `5000/5011` 未停止或重启。

## 浏览器证据

- 用户当前 `5000/login` 刷新后：`.story-backdrop` 高度覆盖首屏，背景图加载完成；`.site-header` 计算为 `background-color: transparent`、`background-image: none`、`backdrop-filter: none`、`box-shadow: none`。
- 截图确认顶部 AI TOKEN 品牌行后方可见代码屏、人物和设备场景，不再出现整条黑色覆盖带。
- `320×844`、`390×844`、`768×900`、`1024×900`、`1440×900`：背景图均存在，品牌栏保持透明，横向溢出均为 `0px`。
- `1440×900` 滚动后：header 进入 `is-scrolled`，仍保持透明、无 blur、无 shadow，横向溢出 `0px`。
- 应用页 console error/warn 数量为 `0`；浏览器宿主 telemetry warning 不计入应用错误。

## 工程与无障碍协议

- 兼容层只负责场景背景定位、尺寸和层级；完整场景动画与交互仍由 `scene-motion.css` 和既有 JS 模块拥有，避免复制业务逻辑。
- 透明品牌行不增加 DOM 节点、焦点目标、事件监听器、网络请求或敏感数据路径。
- `forced-colors`、`prefers-reduced-motion` 原有边界继续生效；场景仍为 `aria-hidden` 装饰层。
- 新增基础 CSS 注释说明兼容场景边界；文件共 480 行，未超过项目 1000 行约束。

## 角色复核

- UI：顶部品牌栏重新回到“场景窗口”角色，背景图可见，文字 keyline 和 signal line 保持扫描性。
- 前端：只补共享基础样式中的场景兼容层，不改模板语义、认证字段、导航和业务脚本。
- 后端：无后端、数据库、接口、认证、CSRF、Key 生命周期或数据流改动。
- 架构师：修复位于 `01-shell` 共享展示边界；复用现有 `.story-backdrop` / `.story-backdrop-image` 结构，不引入新依赖，回滚边界为基础 CSS 兼容块。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
