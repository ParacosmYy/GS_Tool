# UI Accessibility Evidence v153

日期：2026-08-12

## 变更范围

本轮只调整共享 `AI TOKEN` 品牌行的透明边界。目标是消除深色横带观感，同时
保留场景连续透视；移动端还存在完整链桌面选择器覆盖移动规则的问题。

现在 `brand-transparency.css` 与兼容链的 `brand-clarity.css` 统一使用透明场景
窗口契约：

- `background` / `background-image` / `border-bottom` 均保持透明，不绘制横向材质带。
- `backdrop-filter` 与 `box-shadow` 均为 `none`，场景不被二次模糊或阴影覆盖。
- ≤620px 使用同优先级透明覆盖，确保完整链不再被桌面规则污染。
- `hero-topline` 单独保持透明，避免把品牌行和 hero 元数据混成双层横带。
- `forced-colors: active` 继续交给 `Canvas` / `CanvasText` 系统色。

本轮不改变模板语义、认证、导航、接口、数据库、Provider、Key 或数据流。

## 真实浏览器证据

本轮使用项目 `windows/.venv` 启动独立源码验证服务 `localhost:5026`，使用隔离
演示数据库登录。现有 5000/5011 保护服务未重启、未停止、未修改。

| 入口 / 视口 | 场景 | 顶栏计算样式 | 交互 / 横向边界 | 页面日志 |
| --- | --- | --- | --- | --- |
| 5026 / 1440×900 | 登录首帧与 Dashboard `#activity` | `background: transparent`、无背景图、无 blur、无 shadow | Activity 目标 `y=103.7px`，顶栏底 `76px`；`aria-current=location` | 页面业务日志 `[]` |
| 5026 / 390×844 | Dashboard 断点回归 | 同一透明契约，`matchMedia(max-width:620px)=true` | 文档宽 `375px`，`overflow-x=hidden` | 页面业务日志 `[]` |

桌面截图确认品牌行下可见连续场景纹理与设备，文字仍为高对比度 `#f7f8fc`；
移动断点确认完整链不再被桌面选择器覆盖。最终透明实例的业务页面日志为空；
浏览器扩展自身的 telemetry warning 不属于应用日志，未计入应用通过条件。登录表单
Tab 回归可从用户名进入密码字段，焦点顺序未改变。

## 工程与降级

- 兼容链与完整链保留单一材质契约，新增规则不触碰数据或业务状态。
- `prefers-reduced-motion: reduce` 仍关闭顶栏边界动画；材质和文本不依赖动画。
- `forced-colors: active` 下背景和底线由系统色接管。
- 修改后的文本源码均低于 1000 行；无新增依赖。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或
真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
