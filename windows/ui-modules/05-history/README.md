# UI Module 05 — History

负责历史记录、CSV 导出、管理员视图、响应式、键盘/ARIA、日志可读性和运行时性能证据。

主文件：`windows/token_tracker/db.py`、`web.py`、`templates/dashboard.html`、`static/style.css`。

管理员明细必须经过服务端 RBAC；导出必须记录 audit event，并剔除密码、Key、refresh token 和原始敏感 prompt。

