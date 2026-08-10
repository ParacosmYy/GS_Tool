# ADR-022: 共享部署对 Session Secret Fail-Closed

## Status

Accepted

## Date

2026-08-10

## Context

本地体验入口需要零配置，因此缺少 `TOKEN_TRACKER_SECRET_KEY` 时可以生成临时随机 key；
但共享/生产部署若继续使用占位值或每次重启随机 key，会让会话和 CSRF 签名失去稳定安全
边界。管理员网站和同学账号属于需要保护的资产。

## Decision

- `local` 模式缺少密钥时生成内存随机 key，维持个人一键体验；
- `serve --production` 强制进入 `production` 模式并执行 HTTPS 生产预检；可信 HTTP 分享必须显式使用独立的 `--lan-preview` 模式；
- `shared`、`production`、`lan` 模式要求非占位、至少 32 字符的环境密钥，否则启动失败；
- 错误只说明配置规则，不打印实际密钥或 `.env` 内容；
- `production` 模式同时由部署文档要求 HTTPS 和 `TOKEN_TRACKER_SECURE_COOKIE=1`；`lan` 模式仍要求稳定 Secret，但明确承认其仅为可信网络 HTTP 预览。

## Consequences

- 配错共享环境会尽早失败，不会带着可疑 session 签名运行。
- 本机 `start.bat` 不会被强制要求手动配置密钥，但每次进程重启会让本机旧会话失效。
- 生产发布系统必须把密钥作为受保护的环境变量或秘密管理项注入，不提交到仓库。

## Verification

- local 模式可生成 64 hex 字符临时 key。
- shared/production/lan 模式对缺失、占位和过短 key 抛出配置错误；有效 key 才可进入对应服务启动路径。
- `serve --production` 在创建 Flask app 前设置 production 模式并执行严格预检；`serve --lan-preview` 不得复用 production 标志。
