# Web UI 动效与视觉架构

> Author: AI Token Tracker Engineering Team  
> Maintainer: Project Owner  
> Scope: `windows/token_tracker/templates` 与 `static/` 的展示层

## 目标

网页端的动效服务于信息层级和状态反馈，不改变业务流程、接口契约或数据计算。背景插画是品牌装饰层，不能遮挡文字、焦点、图表或点击区域。

## 分层边界

| 层 | 归属 | 责任 | 禁止事项 |
| --- | --- | --- | --- |
| 结构层 | `templates/base.html`、页面模板 | DOM 顺序、标题层级、语义区域、ARIA live 区域 | 不在模板中写大段样式或业务计算 |
| 视觉层 | `style.css`、`ui-polish.css` | 设计 token、布局、色彩和业务表面 | 不读取 API、不保存用户数据 |
| 场景层 | `base.html`、`scene-motion.css`、`static/modules/motion.js` | 显式主图、背景、指针、RAF 调度、滚动 reveal、减弱动效策略 | 不操作 token、Cookie、API 或表单数据 |
| 页面编排层 | `auth.js`、`app.js`、`admin.js` | 调用动效/导航原语、处理页面状态、渲染安全文本 | 不重复实现指针循环、背景物理或章节观察 |
| 品牌资产层 | `static/assets/` | 版本化图片资源和回滚资产 | 不把文字、密钥或业务数据嵌入图片 |

页面脚本只调用 `setupPointerFollower`、`setupBackdropMotion`、`setupSurfaceMotion`、`setupReveal` 和
`setupNavigation`、`setupRangeSwitcher` 等稳定入口；指针/背景/内容动效集中在 `motion.js`，路由与章节位置集中在
`navigation.js`，周期选择器状态与 active pill 几何集中在 `range-switcher.js`，装饰样式集中在 `scene-motion.css` 与
`ui-polish.css`，这样登录页、观测台和管理员页共享同一套规则。

## 动效契约

1. 所有持续动画必须是装饰性的，不能让内容进入 DOM 后仍然不可读。
2. 鼠标跟随使用单一 `requestAnimationFrame` 循环和弹簧插值；卡片只更新 CSS 自定义属性，不触发布局重排。
3. 交互目标获得更大的光环和环形指针，按下时只做短促缩放反馈；原生鼠标、键盘焦点和触摸输入保持可用。
4. `prefers-reduced-motion: reduce` 或非精细指针设备关闭持续跟随、扫光和漂移，页面仍显示完整内容。
5. 加载、成功、错误和空态使用 `data-motion-state`，状态必须同时有文字或 ARIA 信息，不能只依赖颜色。
6. 背景层的遮罩优先保证正文对比度；插画位置、透明度和裁切在 320px、768px、1024px、1440px 宽度都要有明确规则。
7. 固定背景和环境光必须收敛在视口内；鼠标视差通过 `background-position` 或内部属性变化实现，不使用会扩大 `scrollWidth` 的负 inset/整体缩放。
8. 当前路由或章节必须同时通过可见高亮和 `aria-current="location"` 表达；滚动进度线只提供辅助层次，不替代标题、焦点或文字状态。

## 背景版本策略

当前品牌资产为 `embedded-rust-engineer-bg-v14`：左侧保留标题负空间，右侧承载成年、御姐气质且非性化的二次元嵌入式 Rust/RL 工程师、银色 MacBook Pro 风格专业笔记本、紧凑 Mac Studio 风格桌面工作站与 Rust/RL 遥测屏幕。v13、v12、v11、v10、v9、v8、v7、v6、v5、v4、v3、v2 与更早版本保留作回滚参考，不直接参与默认渲染；Windows 与 Android 通过各自静态资源路径消费同一像素资产，端侧遮罩和动效实现保持解耦。详见 ADR-095。

Windows 和 Android 各自复制同一 PNG，并通过 SHA-256 校验保持内容一致；网页只通过
`.story-backdrop` 内的显式 `.story-backdrop-image` 和 CSS 回退引用图片，业务组件不直接依赖
资源文件名。v8 的画面不承载可读文案，避免生成式图片中的伪文字干扰产品信息层。

## 性能与安全边界

- 大图只作为固定装饰层，`pointer-events: none`，不参与命中测试。
- 外部 API 响应仍由页面脚本投影为安全文本；动效层不接收 provider 原始响应。
- 不在 `localStorage` 保存 API Key、密码、access token 或 refresh token。
- 新增动效不得引入第三方运行时依赖；优先使用 CSS、DOM 和现有模块。
- 若动效模块需要跨页面共享新状态，先扩展稳定原语，再由页面编排层调用，不把页面条件分支塞进共享循环。

## 验收清单

- 登录页首屏能看到正面设备屏幕，标题区域仍有足够对比度。
- 鼠标移动、悬停按钮、按下按钮和离开窗口时没有残留动画状态。
- 键盘 Tab、焦点环、屏幕阅读器文本与鼠标动效互不依赖。
- 页面无 JavaScript 语法错误；健康接口、登录页和静态资源返回成功。
- 减弱动效模式下不出现闪烁、不可读文本或隐藏内容。
