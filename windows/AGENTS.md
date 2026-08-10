# Windows 中心服务约束

根目录 `AGENTS.md` 是本项目总约束；本文件只补充 Windows 端的运行边界。

- `windows/` 是中心服务、网页、CLI 和数据库的唯一宿主；Android 与浏览器都通过 HTTP API 访问。
- `data/token_tracker.sqlite3` 只由服务端读写；客户端不得读取、复制或上传 SQLite 文件。
- 本机体验默认 `127.0.0.1`；团队分享必须使用生产 WSGI 服务、HTTPS、持久化磁盘和明确的管理员 RBAC。
- 所有管理员查询、导出、角色变更都要写入审计事件；响应中不得包含密码哈希、API Key、refresh token 或原始敏感 prompt。
- `token_tracker/web.py` 只做路由/会话编排；权限、事件、管理员聚合和数据访问按文件规模门禁拆分。
- 项目级技能位于 `windows/skills/`；浏览器 UI 变更必须读取 `project-ui-orchestration/SKILL.md`，并按技能索引留下截图、DOM、控制台和响应式证据。
