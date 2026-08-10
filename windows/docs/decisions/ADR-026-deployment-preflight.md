# ADR-026: 增加无副作用部署预检

## Status

Accepted

## Date

2026-08-10

## Context

共享部署涉及 Session Secret、Secure Cookie、provider allowlist、SQLite 路径和 HTTPS
边界。单靠 README 容易把“能启动”误认为“可安全分享”，而真正启动服务或改防火墙不适合
作为自动检查的副作用。

## Decision

新增 `python -m token_tracker preflight`：

- 默认执行非生产配置检查；
- `--production` 进入 production mode，并要求 Secure Cookie、provider HTTPS allowlist
  和有效 Session Secret；
- 检查数据库不在静态资源目录；
- 只读环境和路径检查，不联网、不写数据库、不改防火墙、不创建云资源；
- 失败返回非零退出码，消息不输出密钥或数据库内容。
- `serve --production` 在创建 Flask 应用或启动 Waitress 前复用同一套 production 预检，不能绕过该门禁直接监听端口。

## Consequences

- 发布脚本和人工部署可以在启动前获得确定的配置门禁。
- 预检不能证明 DNS、证书、Windows 防火墙、反向代理或恢复时间；这些仍需实际部署演练。

## Verification

- local 预检可通过；缺密钥、HTTP provider 或不安全 cookie 的 production 预检失败。
- CLI help、Python compile、health 和备份验证保持通过。
