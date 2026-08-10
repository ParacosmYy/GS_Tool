# Web UI-3 响应式与可访问性证据

**作者：** AI Token Tracker Engineering Team  
**日期：** 2026-08-10  
**范围：** Windows 登录页、场景背景层、认证表单、窄屏布局

## 执行方式

使用本机 5011 端服务和 Chrome 151 的 DevTools Protocol 设备指标覆盖执行运行时检查。当前环境未提供 Chrome DevTools MCP，因此使用等价的本地 CDP 连接；只访问 `127.0.0.1`，不读取 Cookie、localStorage、密码或令牌。

## 320px 运行时结果

| 检查项 | 结果 |
| --- | --- |
| CSS viewport | `innerWidth=320`，`innerHeight=900` |
| 可视布局宽度 | `clientWidth=305`；差值来自垂直滚动条 |
| 横向溢出 | `body.scrollWidth=305`、`documentElement.scrollWidth=305`，通过 |
| 认证布局 | shell `275px`，卡片 `275px`，均在可视布局范围内 |
| 标签关联 | `用户名 → username`、`密码 → password`，通过 |
| 焦点序列 | `username → password → submit → Create one → 隐私与数据说明`，通过 |
| reduced motion | `prefers-reduced-motion: reduce` 命中；背景和轨道动画计算值为 `none` |
| 浏览器运行时 | 登录页无 console exception、无 failed request、无 4xx/5xx response |

此前 320px 检查发现背景 fixed 层与认证页装饰 pseudo-layer 把文档扩大到 340px。本切片通过窄屏背景边界收敛和认证装饰层 `overflow: clip` 修复了具体来源，没有使用全局遮罩掩盖业务内容溢出。

## 语义与对比度结果

- 页面存在一个 `h1` 和一个认证 `h2`；背景、轨道和鼠标层均标记 `aria-hidden="true"`。
- 用户名、密码均使用可见 `label` 包裹控件；提交按钮为原生 `button`。
- 运行时颜色 token 的保守语义组合对比度：标题/画布 `18.76:1`、正文/画布 `14.37:1`、label/panel `13.19:1`、卡片标题/panel `17.22:1`、主按钮文字/按钮 `16.31:1`、placeholder/input `9.64:1`。
- 这些数值基于运行时 computed color 和语义面板色；背景插画仍由遮罩层和表面层隔离，不能把图片中的局部像素当作正文承载面。

## 截图证据

- [`ui-audit-v4-320.png`](../.cache/ui-audit-v4-320.png)：CDP 设备指标覆盖后的窄屏登录页。
- [`ui-audit-v4-768.png`](../.cache/ui-audit-v4-768.png)：移动/平板断点登录页。
- [`ui-audit-v4-1024.png`](../.cache/ui-audit-v4-1024.png)：平板/桌面过渡断点登录页。
- [`ui-audit-v4-1440.png`](../.cache/ui-audit-v4-1440.png)：桌面登录页。

## v6 背景与四档回归

- v6 资源已完成本地视觉检查：保持左侧登录负空间、右侧单角色构图，并强化笔记本、桌面工作站、Rust/RL telemetry 屏幕与服务器机架的层次。
- CSS 已切换到 `/static/assets/embedded-rust-engineer-bg-v6.png`；v5 资产仍保留，可在不改动业务模板的情况下回滚。
- v6 仅有图像生成器和本地文件检查证据；当前环境没有可用浏览器 CDP，因此没有把旧 v5 截图冒充 v6 运行时证据。
- 下一次具备浏览器 CDP 运行时后，需要重新生成 `ui-audit-v6-1440/1024/768/320.png`，并复核四档横向溢出、空态、焦点和局部对比度。
- 本轮静态语义切片已通过：仪表盘只保留一个 `h1`、记录表列头和管理员详情表使用 `scope="col"`、表格有 caption，管理员详情打开后焦点进入关闭按钮并在关闭后返回触发按钮；登录/注册密码 maxlength 与服务端 256 字符边界一致。
- v4 截图保留为历史基线，不再作为 v6 默认渲染证据。

## v6.1 场景层增强（源码与空 schema smoke）

- 登录/注册页的 v6 角色场景增加 `RUST / RL WORKBENCH` 与
  `MACBOOK / STUDIO SCENE` 的低频场景签名；它标记为 `aria-hidden`，不伪装成真实
  硬件连接状态，也不参与业务状态判断。
- 认证卡片改为受控透明度并启用 `backdrop-filter`，让工作台插画可见，同时保留深色
  内容承载面、正文 token 和输入焦点边界；不支持滤镜的浏览器仍使用不透明度回退。
- 320px 额外收紧签名字距和字号，避免场景标签导致横向溢出；`prefers-reduced-motion`
  会关闭 beacon 脉冲。
- 使用隔离空 schema 的 Flask 渲染 smoke 检查 `/login`、`/register`、`/privacy` 均为
  `200`，v6 背景与场景签名均存在；没有创建真实用户或写入项目数据库。
- 透明卡片、pointer follower、背景视差和 reduced-motion 规则保持不变；下一次具备浏览器
  CDP 运行时后需要重拍 v6.1 四档截图并复核透明卡片后的局部对比度。

## 尚未关闭的门禁

本证据只覆盖无需创建测试账户的登录运行时。仪表盘、连接、历史和管理员页面仍需在合法认证会话中分别完成截图、空态、错误态和键盘回归；在没有用户提供可用体验账号且项目禁止创建测试数据的情况下，不能把模板静态检查冒充为受保护页面运行时通过。UI-3 总闸门因此继续保持进行中。
