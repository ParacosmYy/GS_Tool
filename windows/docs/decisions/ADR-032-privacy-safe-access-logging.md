# ADR-032：应用与 HTTPS 边缘的隐私安全访问日志

## 状态

已接受（2026-08-10）。

## 背景

平台同时存在 Flask/Waitress 应用日志和未来的 Caddy HTTPS 边缘日志。请求可能包含
Session Cookie、Android Authorization、查询参数和同学的访问地址；provider API Key
则只应存在于 POST body 的短生命周期内。默认的通用访问日志格式如果直接输出完整 URL、
请求头或客户端地址，会增加凭据、隐私和排障数据的泄露面。

## 决策

1. 应用层通过 `token_tracker/access_logging.py` 只记录 method、path（不含 query）、
   status、duration_ms 和 request_id；不记录请求头、请求体、Cookie、User-Agent、Key、
   用户 IP 或响应体。该模块同时给 Werkzeug 内置 request-line 加过滤器，避免默认开发
   访问日志重新输出完整 query。
2. Caddy 示例配置使用 `format filter` 显式删除 Authorization、Proxy-Authorization、
   Cookie、Set-Cookie 和 User-Agent，按 `/16`、`/32` 掩码记录 IPv4/IPv6 地址，并删除
   当前所有业务查询参数。新增查询参数必须同步审查过滤器。
3. Caddy 默认敏感请求头脱敏属于额外保障，不替代示例中的显式过滤；禁止在全局配置启用
   `log_credentials`，除非经过单独安全评审。
4. 应用层和 Caddy 日志文件都必须使用受限 ACL、轮转和保留策略；正式 HTTPS、Caddy
   `validate`、ACL 和真实日志样本审查仍是部署环境门禁，不在源码中冒充已完成。

## 取舍

- 访问日志仍能用 request ID、路径和耗时定位请求，但不再提供原始 IP、User-Agent 或
  查询参数分析；需要这类数据时必须经隐私评审后单独加入脱敏字段。
- 查询参数过滤是 fail-closed 的当前契约，新增参数如果未进入过滤器会被部署审查拦截。
- 应用日志与边缘日志仍可能由不同进程输出，部署时必须分别验证，而不能只检查其中一层。

## 验证

- `GET /api/health?api_key=secret-value` 的应用访问日志只包含 `/api/health`、状态、
  耗时和 request ID，未出现 query 或 secret。
- Python 编译和文件行数门禁通过。
- Caddy 配置只作为模板保存；正式机器上的 `caddy validate`、HTTPS 访问和真实日志脱敏
  样本仍待具备 Caddy、域名/证书和运维权限后执行。

## 参考

- Caddy 官方 `log` 指令与 `format filter` 文档：
  <https://caddyserver.com/docs/caddyfile/directives/log>
