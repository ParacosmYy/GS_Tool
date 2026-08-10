# DEV-1 Shell / Auth

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**职责：** 全局布局、登录注册、错误页、导航和认证状态的网页实现。

## 输入与输出

- 输入：已批准的 API/权限契约、UI token 和认证状态。
- 输出：`base.html`、认证模板、入口脚本及相应浏览器证据。

## 硬约束

- 不在模板中实现授权判断；权限必须由服务端强制。
- 不把 API Key、访问令牌或密码写入 localStorage、日志或 HTML。
- 表单必须提供键盘焦点、错误语义、加载状态和可见对比度。
- 单文件不超过 1000 行；跨模块能力通过稳定接口暴露。

