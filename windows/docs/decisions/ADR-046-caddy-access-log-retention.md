# ADR-046：Caddy access log 脱敏与保留策略

## 状态

已接受（2026-08-10）。

## 问题

边缘日志既要支持故障排查，又不能无限增长或保留精确客户端地址、Cookie、认证头和
不必要的请求信息。依赖 Caddy 默认滚动值会让团队无法从配置审计实际保留窗口。

## 决策

- `windows/deployment/Caddyfile.example` 使用 Caddy 官方 `log` file writer 配置：每天
  `00:00` 或达到 `100MiB` 时滚动，最多保留 14 个文件和 14 天，并使用本地时间命名。
- `format filter` 删除 Authorization、Proxy-Authorization、Cookie、Set-Cookie、User-Agent，
  遮罩 remote/client IP，并删除已知统计 query 参数；应用层仍保留独立的脱敏 request-id 日志。
- 日志目录必须由部署负责人预先创建并通过 ACL 预检；Caddy 示例不会自动创建目录、修改权限、
  申请证书或启动服务。
- 正式环境必须使用部署主机实际安装的 Caddy 执行 `preflight-edge.ps1` 和 `caddy validate`，
  再提供真实轮转/权限样本；本地源码检查不能代替现场证据。

## 官方依据

- [Caddy log directive](https://caddyserver.com/docs/caddyfile/directives/log)：file writer 的
  `roll_at`、`roll_size`、`roll_keep`、`roll_keep_for`、`roll_local_time` 和 filter/query/ip_mask 语法。
- [Caddy reverse_proxy directive](https://caddyserver.com/docs/caddyfile/directives/reverse_proxy)：
  `127.0.0.1:5000` upstream 地址格式。

## 验证

- 官方文档核对了当前示例的 `filter`、`query`、`ip_mask`、file rotation 和 reverse proxy 语法。
- 本机未安装 Caddy，因此没有声称 `validate` 或真实 ACL/轮转已通过。
