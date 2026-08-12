# UI Accessibility Evidence v149

日期：2026-08-12

## 变更范围

本轮修复登录/注册页首帧的自动聚焦层级。桌面端为了缩短键盘路径会自动聚焦
用户名输入框，但浏览器恢复焦点或脚本焦点会被 `:focus-visible` 识别为用户
意图，首帧直接显示强 lime 输入光环和整张卡片的 lime `:focus-within` 边界，
视觉上容易被误读为错误态。

现在自动聚焦状态使用 `data-auth-autofocus` 作为显式 presentation contract：
输入框保持可输入但只显示 quiet lavender edge，卡片同步降低为低强度 lavender
边界；首个 pointer/keyboard 事件会取消该标记并恢复完整键盘焦点反馈。若用户在
首帧 `requestAnimationFrame` 前操作，异步自动聚焦不会重新写回标记。

本轮不改变表单字段、认证接口、会话、CSRF、数据库、导航或业务数据流。

## 真实浏览器证据

本轮使用项目 `windows/.venv` 启动独立源码验证服务 `localhost:5026`。现有
5000/5011 保护服务未重启、未停止、未修改；5011 当前仍运行旧兼容样式链，
因此只作为保护端口安全基线，不将其误记为新源码运行证据。

| 入口 / 视口 | 页面 | 自动聚焦 | 输入框首帧边界 | 卡片首帧边界 | 顶栏 | 文档溢出 | 页面日志 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 5026 / 1440×900 | 登录 | `true` | quiet lavender / 无强光环 | lavender 低强度 | transparent / no shadow / no blur | 无 | `[]` |
| 5026 / 390×844 | 登录 | 不启用 | 原有 50px 控件高度 | 原有移动几何 | transparent / no shadow / no blur | 无 | `[]` |
| 5026 / 1440×900 | 注册 | `true` | quiet lavender / 无强光环 | lavender 低强度 | transparent / no shadow / no blur | 无 | `[]` |
| 5026 / 390×844 | 注册 | 不启用 | 原有 50px 控件高度 | 原有移动几何 | transparent / no shadow / no blur | 无 | `[]` |

源码实例实测登录页桌面卡片保持 `440px` 宽，注册页保持 `596px` 宽；两页
390px 卡片均保持 `344.67px` 宽，顶栏保持 `344.67px` 宽。首帧截图确认
主标题、场景插画和登录卡之间的层级恢复，输入框仍具备可见插入光标。

## 可访问性与降级

- 自动聚焦只改变首帧视觉强度，不改变焦点顺序、输入语义或键盘可达性。
- 用户开始 pointer/keyboard 交互后，`:focus-visible` 恢复完整 lime 焦点环；
  焦点反馈不依赖鼠标或持续动画。
- `prefers-reduced-motion: reduce` 仍关闭认证场景动效；`forced-colors: active`
  继续由 `Highlight` / `Field` 系统色接管，不引入 lavender 颜色。
- 登录/注册桌面与移动端均无横向溢出；修改后 `auth.js` 与 `auth-focus.css`
  均低于 1000 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或
真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
