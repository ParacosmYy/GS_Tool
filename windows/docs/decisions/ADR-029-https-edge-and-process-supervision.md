# ADR-029：HTTPS 边缘与 Windows 生产进程启动边界

**状态：** Accepted  
**日期：** 2026-08-10  
**范围：** Caddy、Waitress、`deployment/start-production.ps1`、Windows 部署 runbook

## 背景

正式分享需要稳定域名、HTTPS、生产 WSGI 和可回滚的中心服务。仅提供一个 Caddy 反代片段或手动运行 Flask，容易把证书、边缘安全头、生产预检和进程生命周期混在一起，无法形成可审计的启动链。

## 决策

1. Caddy 负责公网 HTTPS、压缩、边缘安全响应头和 JSON 访问日志；上游只连接 `127.0.0.1:5000`，
   `serve --production` 同样只允许 Waitress 绑定 loopback（详见 ADR-061）。
2. Waitress 负责 Python WSGI 服务；`start-production.ps1` 只负责执行 production preflight 和启动 `serve --production`，不申请证书、不改防火墙、不创建 Windows 服务。
   根目录个人体验入口也复用已锁定的 Waitress，不再把正常体验路径交给 Flask development server；
   个人入口仍只监听 loopback，并独立保留端口回退策略。
3. Caddy 配置示例使用正式域名占位符，禁止原样用于生产；日志目录由部署负责人预先创建并设置 ACL，
   ADR-032 的显式 `format filter` 负责删除敏感请求/响应头和当前查询参数；不能把 API Key 放进 URL 或日志字段。
4. 生产启动必须同时通过应用 production 预检和 Caddy `validate`；任一失败都不得监听或对外发布。
5. 回滚先停流量和 Waitress，再验证备份、隔离恢复、负责人确认，最后显式替换目标并重新执行 preflight/health；不自动复制或覆盖当前数据库。

## 验收边界

- 当前已完成配置模板、启动包装器、命令和回滚顺序文档。
- 当前环境没有获得真实域名、证书、Caddy 安装和 Windows 服务管理权限，因此没有声称 Caddy validate、HTTPS 外部访问或恢复时间目标演练已通过。
