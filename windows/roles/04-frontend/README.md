# 前端工程师交付包

**作者：** AI Token Tracker Engineering Team  
**职责：** 将契约和三个 UI 角色的约束落成可维护的模板、原生 JavaScript、CSS 和图表交互。

## 输入

- `docs/api-contract.md`
- `docs/ui-motion-spec.md`
- `roles/01-ui-visual/`、`roles/02-ui-motion/`、`roles/03-ui-accessibility/`

## 输出

- `token_tracker/templates/` 页面结构；
- `token_tracker/static/app.js` 浏览器状态和 API client；
- `token_tracker/static/modules/api-client.js`、`charts.js`、`motion.js` 分别承载传输、图表和动效边界；
- `token_tracker/static/style.css` 设计 token、响应式和动效实现。

## 边界

前端不访问 SQLite、不保存认证或 provider secret、不凭客户端数字伪造 token；用户输入用 textContent/模板转义输出，异步操作提供 loading、success、error 和 empty 状态。
