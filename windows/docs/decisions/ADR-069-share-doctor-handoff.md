# ADR-069：分享部署一命令预检边界

**作者：** AI Token Tracker Engineering Team  
**状态：** accepted  
**日期：** 2026-08-10  
**范围：** Windows LAN preview / HTTPS production handoff

## 背景

项目已经分别提供应用部署预检和 Caddy edge 预检，但分享负责人需要记住多个命令，
容易把 LAN HTTP 预览误当生产 HTTPS，或遗漏 Session Secret、数据库路径、日志 ACL
和 Caddy 配置检查。

## 决策

- 新增 `deployment/share-doctor.ps1`，通过 `-Mode LanPreview|Production` 组合已有只读检查。
- LAN 模式固定按 `RUNTIME_MODE=lan` 校验 Secret 和绑定地址，并明确下一步仍需输入 `SHARE`；
  它不打开防火墙，也不启动服务。
- Production 模式固定 loopback，并要求显式提供 Caddyfile 与日志目录，再调用 edge preflight；
  Caddy、证书、ACL、Waitress 和外部域名仍由部署负责人执行。
- 脚本临时设置运行模式环境变量后恢复原值，不读取或打印 Secret，不创建数据库、备份或日志目录。

## 验证与边界

- PowerShell 脚本采用项目既有 `preflight-edge.ps1` 和 CLI `preflight`，源码/文本行数和作者头纳入发布审计。
- 当前环境没有 Caddy，因此 Production 真实 edge 预检仍保持 pending；LAN 预检只证明配置边界，
  不证明网络可信度、防火墙、真实同学访问或 HTTPS。
