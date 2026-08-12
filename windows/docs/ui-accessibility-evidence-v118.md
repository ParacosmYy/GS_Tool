# UI 可访问性与视觉证据 · v118

## 变更主题

将登录/注册认证字段从连续的矩形输入区收敛为可扫描的 signal lane。新增 `auth-signal.css`，为共用认证卡片提供低 alpha 深蓝阅读面、字段局部 signal line、字段元信息和更轻的移动端材质；模板只增加视觉辅助编号，`IDENTITY / 01` 与 `ACCESS / 02` 使用 `aria-hidden="true"`，不改变输入可访问名称、认证接口、CSRF、字段值或数据流。

隔离实例：`127.0.0.1:5219`，运行目录为 `windows/.cache/ui-v118-runtime-20260812-5219`。受保护服务 `5000/5011` 未触碰。

## 浏览器证据

- `1440×900` 登录首屏：认证卡片使用低 alpha 深蓝透景层，标题、说明、输入和主按钮维持稳定阅读层；桌面自动聚焦仍落在用户名字段。
- `390×844` 登录首屏：字段右侧显示 `IDENTITY / 01`、`ACCESS / 02`，卡片与背景场景保持连续；输入控件仍保持至少 50px 高度，主按钮保持 48px 触控高度。
- 用户名获得真实焦点后：父级 label `:focus-within` 命中，局部 signal line `opacity=0.9 / scaleX(1)`，`IDENTITY / 01` 变为 signal lime；输入保留原有键盘 outline 和焦点阴影。
- `/register`：共用样式正确继承，两个字段辅助编号存在；页面保持 `1 个 h1 / 1 个 h2`。
- `320/390/768/1024/1440`：五档均无横向溢出；文档 `scrollWidth` 与 `clientWidth` 一致。
- 当前控件可访问名称仍为“用户名”和“密码”，辅助编号均为装饰性 `aria-hidden`；应用页 console error/warn 数量为 `0`。

## 动效与无障碍协议

- 信息：字段下方 signal line 表达“当前输入通道”，字段编号表达认证流程的视觉顺序，不伪造业务状态。
- 触发：`:focus-within` 驱动局部边界，持续时间为 `.3s/.45s`，只改变 opacity/transform/color，不引发布局测量或网络请求。
- 降级：`prefers-reduced-motion: reduce` 关闭本模块过渡；`forced-colors` 隐藏装饰 signal line，使用系统字段与边界颜色。
- 输入：仍使用原生 `<label>`、`<input>`、`button` 和既有 focus-visible contract；编号标记不进入屏幕阅读器名称。

## 角色复核

- UI：认证字段获得明确的层级、流程序号与焦点反馈，减少大块深色材质和连续矩形带来的阅读疲劳。
- 前端：新增 `auth-signal.css`，登录/注册模板只复用稳定字段结构；没有新增业务脚本、API、依赖或密钥存储。
- 后端：无后端、数据库、接口、认证逻辑或 CSRF 处理改动。
- 架构师：职责限定在 `02-auth`，样式模块低耦合、低于 1000 行，forced-colors/reduced-motion 和回滚边界明确。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
