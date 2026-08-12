# UI 可访问性与视觉证据 · v117

## 变更主题

修复最上方 `AI TOKEN / OBSERVATORY` 品牌栏的黑色覆盖错觉。新增 `brand-transparency.css` 作为最后加载的 01-shell 透明契约：普通配色下品牌栏及其子级保持透明、无背景图、无 blur、无 shadow；文字 keyline 和滚动 signal line 继续提供必要的阅读边界。forced-colors 仍交给系统 Canvas 颜色，不改变模板、认证、导航、API 或业务数据。

隔离实例：`127.0.0.1:5218`，运行目录为 `windows/.cache/ui-v117-runtime-20260812-5218`。受保护服务 `5000/5011` 未触碰。

## 浏览器证据

- `1440×900` 登录首屏：背景插画连续穿过品牌栏，品牌行没有黑色底带；登录卡片和首屏场景层级保持不变。
- `1440×900` 滚动状态：sticky 品牌栏仍为透明窗口，滚动进度线作为单独 1px signal boundary 保留，未形成第二条深色横带。
- `390×844`：品牌标记、`AI TOKEN` 和 `OBSERVATORY` 副标题保持在同一透明场景中，认证卡片排版未发生横向挤压。
- `320/390/768/1024/1440`：五档均无横向溢出；文档 `scrollWidth` 与可用 `clientWidth` 一致。
- 最终计算样式：普通配色下 header `background-color: rgba(0, 0, 0, 0)`、`background-image: none`、`backdrop-filter: none`、`box-shadow: none`；滚动态与首屏一致。
- 应用页 console error/warn 数量为 `0`；浏览器宿主 telemetry warning 不计入应用错误。

## 动效与无障碍协议

- 信息：透明栏表达品牌 chrome 与背景场景属于同一视觉层；细 signal line 只表达滚动位置，不承载业务状态。
- 触发：沿用现有 sticky header 与滚动进度逻辑，本轮没有新增监听器、计时器、网络请求或布局测量。
- 降级：`prefers-reduced-motion` 关闭本模块新增的阴影过渡；`forced-colors` 使用 `Canvas/CanvasText`，移除文字阴影和装饰竞争。
- 输入：品牌栏仍保留原有链接、导航和键盘 focus-visible；子级透明规则不改变 Tab 顺序、点击区域或 DOM 语义。

## 角色复核

- UI：移除顶部黑色覆盖观感，保留轻量 hairline 与文字轮廓，避免透明场景被横向切断。
- 前端：新增 `brand-transparency.css` 并在 `base.html` 最后接入；没有触碰业务脚本、认证流程、接口或数据状态。
- 后端：无后端、数据库、接口或密钥处理改动。
- 架构师：透明契约独立于旧视觉模块，使用明确的跨模块边界；新增文件低于 1000 行，forced-colors/reduced-motion 与回滚路径清晰。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
