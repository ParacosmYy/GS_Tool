# ADR-078：个人体验入口采用 loopback fail-closed

**作者：** AI Token Tracker Engineering Team  
**状态：** Accepted  
**日期：** 2026-08-10  
**范围：** Windows 本机体验入口、根目录 Python 入口、数据访问边界

## 背景

根目录 `run.py` 与 `windows/run.py` 是项目最容易被直接双击或复制执行的入口。它们默认服务个人体验，并可能读取本机 token 使用日志。若入口直接信任 `TOKEN_TRACKER_HOST`，用户可能在不了解网络边界的情况下配置成 `0.0.0.0`，导致本地账本被局域网访问。

项目已经把局域网预览定义为独立命令边界：`python -m token_tracker serve --lan-preview`。个人入口不应复用该边界，也不应因为环境变量而隐式改变暴露范围。

## 决策

个人体验入口对主机地址采用 fail-closed 策略：

- 未设置、空值或 `localhost` 统一规范化为 `127.0.0.1`。
- 仅接受 IPv4 loopback 地址；当前拒绝 IPv6 是因为该入口的端口探测与 Waitress 绑定合同使用 IPv4 socket。
- 非 loopback、无法解析的地址，以及 IPv6 地址均在创建应用和打开数据库前拒绝启动。
- 需要局域网访问时，必须显式使用 `--lan-preview`，并遵循局域网/HTTPS 部署文档；不得通过 `TOKEN_TRACKER_HOST` 绕过确认边界。

## 后果

个人入口的默认数据暴露面保持为本机，错误配置会快速失败并给出可操作提示。代价是用户不能通过根目录入口直接绑定局域网地址；这是有意的职责分离，避免个人入口与共享入口产生隐式行为差异。

## 验证与回滚

验证要求：

1. `resolve_local_host("localhost")` 与 `resolve_local_host("127.0.0.1")` 返回 `127.0.0.1`。
2. `0.0.0.0`、非 loopback 地址、无效地址和 IPv6 地址被拒绝。
3. 根目录入口在空闲端口上仍能返回 `/api/health` 的 `200`。
4. 静态发布审计、Python 编译、差异空白检查和文件行数检查通过。

若未来需要支持 IPv6 本机体验，应先为 socket 探测、Waitress 绑定、浏览器 URL 和发布文档建立独立合同，再单独修订本 ADR；不得通过放宽当前校验临时实现。

