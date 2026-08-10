# UI Module 02 — Auth

负责登录、注册、错误页和认证表单的入场、聚焦、错误、成功与可访问性状态。

主文件：`windows/token_tracker/templates/login.html`、`register.html`、`error.html`、`windows/token_tracker/static/auth.js`。

认证逻辑仍由 Windows 后端负责；动效代码不得读取密码、修改 CSRF 或保存凭据。

