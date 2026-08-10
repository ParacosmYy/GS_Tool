# ADR-034：可信局域网分享只保留一个确认入口

## 状态

已接受（2026-08-10）。

## 背景

个人体验入口必须只监听 `127.0.0.1`，局域网分享才允许绑定 `0.0.0.0`。原先
`windows/start-lan.bat` 和 `deployment/start-lan-preview.bat` 都能启动 LAN 服务，
但前者没有显式确认，导致同一项目存在两套暴露策略，用户可能在不知情时把服务开放到局域网。

## 决策

1. `deployment/start-lan-preview.bat` 是唯一 LAN 启动实现，必须输入 `SHARE` 才继续。
2. `windows/start-lan.bat` 只作为兼容 wrapper，委托到 canonical 脚本，不复制 Python、端口、
   Waitress 或安全提示逻辑。
3. LAN 模式仍固定为可信网络临时 HTTP 预览；公网分享必须走 HTTPS、生产 WSGI、持久化磁盘、
   限流和备份门禁。

## 结果

- 个人体验和局域网分享的权限边界清晰，策略只维护一份。
- 旧的 `windows/start-lan.bat` 路径仍可用，但会先显示风险并要求明确确认。
- 此决策不自动改防火墙、不检测网络可信度，也不替代正式 HTTPS 部署。

## 验证

- wrapper 内容只包含委托和错误码传递，没有第二份 `0.0.0.0` 启动命令。
- canonical 脚本仍保留 `SHARE` 确认、Waitress LAN 预览和明文 HTTP 风险提示。
