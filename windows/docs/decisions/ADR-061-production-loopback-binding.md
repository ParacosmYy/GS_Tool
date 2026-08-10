# ADR-061：生产 WSGI loopback 绑定边界

## 状态

已接受（2026-08-10）。

## 背景

正式部署由 Caddy 终止 HTTPS 并反代到 Waitress。若 `serve --production` 允许通过命令行或环境变量
绑定 `0.0.0.0`，应用端口就可能绕过 Caddy 的 HTTPS、访问日志过滤和边缘安全头，直接暴露在网络上。
LAN 预览是不同的显式用户确认场景，不能与生产模式复用同一放宽规则。

## 决策

- `serve --production` 和 `preflight --production` 要求 `TOKEN_TRACKER_HOST`/有效命令行绑定地址
  为 `localhost`、`127.0.0.1`、`::1` 或其他 loopback IP；非 loopback 地址在监听前拒绝。
- `preflight --production --host <address>` 支持直接预检命令行地址；若同时传入 `--host`，它覆盖
  环境变量并必须与后续 `serve --production --host` 使用同一个值。
- `serve --lan-preview` 不受该 production loopback 门禁影响，但必须走显式 `SHARE` 确认、Waitress
  和 LAN 模式的 Secret 预检；它不被视为 HTTPS 生产部署。
- 命令行 `--host` 在 production 预检中使用与最终 Waitress 监听相同的有效值，避免“预检检查环境变量、
  实际监听另一个地址”的配置漂移。

## 取舍

生产进程不能直接作为局域网 HTTP 服务使用；需要分享时必须完成 Caddy/HTTPS edge 部署。这样把
TLS、日志和边缘 ACL 保持在一个可审计入口，降低误配置导致的旁路暴露风险。

## 验证边界

- 已完成：loopback/non-loopback 配置分支 smoke、Python 编译和只读发布审计。
- 待完成：正式 Caddy 主机上的证书、ACL、外部 HTTPS 和 Windows 进程监督演练。
