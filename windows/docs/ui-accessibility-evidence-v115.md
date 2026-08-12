# UI 可访问性与视觉证据 · v115

## 变更主题

为透明 AI TOKEN 品牌行增加独立 `header-chrome.css` 模块。新增内容只负责一条 composited hairline、品牌标记的轻量 hover/focus 反馈和文字 keyline；顶栏本体继续保持 `background: transparent`、`background-image: none`、无 `backdrop-filter`、无 `box-shadow`，不把透明场景重新做成黑色面板。

隔离实例：`127.0.0.1:5216`，运行目录为 `windows/.cache/ui-v115-runtime-20260812-5216`。受保护服务 `5000/5011` 未触碰。

## 浏览器证据

- 登录页 `1440×900`：header `1320×76px`，`background-color=rgba(0,0,0,0)`、`backdrop-filter=none`、`box-shadow=none`；顶部 hairline 为独立 `1px` 伪元素，渐变不参与布局。
- 登录页 `390×844`：body `scrollWidth=375/clientWidth=375`，无横向溢出；透明 header 仍为 `344.667×76px`，进度轨道宽度为 `344.667px`，hairline 正常渲染。
- Dashboard 深滚至 `scrollY=2360`：header 自动进入 `is-scrolled`，仍保持透明、无 blur、无 shadow；hairline `display=block`，滚动进度线 `width=1320px`、`transform=matrix(.788,0,0,1,0,0)`，顶部插画与设备仍可见，没有黑色横带。
- 登录页与 Dashboard 截图确认 AI TOKEN 品牌标记、导航、认证卡片和深滚表单之间的层级连续；本轮没有新增 DOM、API、业务状态或密钥处理。

## 动效协议

- 信息：hairline 表达 sticky 品牌行的空间边界；品牌圆环 hover/focus 表达当前可交互目标；不表达业务状态。
- 触发：hairline 仅在允许动效时以 `8s ease-in-out` 呼吸透明度/横向尺度；品牌标记只在 pointer hover 或键盘 focus 时以 `transform` 和 composited shadow 反馈，不改变布局。
- 性能：使用伪元素、`opacity`、`transform` 和既有 `scaleX` 进度轨道；不读取布局、不触发布局、不新增事件监听器。
- 降级：`prefers-reduced-motion: reduce` 关闭 hairline 与品牌标记过渡；`forced-colors: active` 使用 `Canvas/CanvasText`，移除装饰阴影并保留边界。
- 设备：窄屏验证保持透明顶栏和原生滚动；触摸屏不依赖 hover，键盘仍使用原有 `:focus-visible` 焦点环。

## 角色复核

- UI：透明品牌行拥有轻量光学边界，不增加黑色填充、模糊层、第二导航卡片或无意义装饰。
- 前端：新增 `header-chrome.css`，通过 `base.html` 的稳定 stylesheet contract 接入；共享滚动模块与业务脚本不变。
- 后端：无后端、数据库、接口或密钥处理改动。
- 架构师：品牌 chrome 从通用页面表面中拆出高内聚模块；文件行数低于 1000 行，依赖方向与回滚边界清晰。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
