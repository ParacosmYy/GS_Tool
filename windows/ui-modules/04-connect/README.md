# UI Module 04 — Connect

负责 Base URL、API Key 内存态、模型检测、自动代理、usage 反馈、错误/限流/超时和异常补录。

主文件：`windows/token_tracker/templates/dashboard.html`、`windows/token_tracker/static/app.js`、`static/modules/api-client.js`、`windows/token_tracker/providers.py`。

禁止在日志、localStorage、SQLite、CSV、DOM 文本或错误消息中泄露 Key。

