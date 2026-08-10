# Windows 中心网站部署

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**用途：** 让同学通过一个 HTTPS 网站使用同一中心数据库。

## 推荐拓扑

```text
同学浏览器 / Android
          │ HTTPS
          ▼
      Caddy/Nginx
          │ 127.0.0.1
          ▼
  Waitress → Flask API v1 → SQLite
```

Windows 电脑只需要运行一个中心服务；同学不安装 Python、不接触 SQLite，也不需要知道项目目录。Android 只配置同一个 HTTPS Base URL。

## 本机/局域网体验

根目录 `start.bat` 仍是个人体验入口。它默认使用 5000；如果旧的本机服务已经占用
5000，当前源码会自动选择 5001–5020 中的可用端口，不会终止旧进程。可信局域网临时分享可以使用：

```powershell
cd D:\Workplace\Agent_Workplace\ai-token-tracker\windows
python -m token_tracker serve --host 0.0.0.0 --port 5000 --lan-preview
```

这不是公网生产部署；需要配置 Windows 防火墙、备份和可信网络边界。

为了避免把根目录个人体验入口意外暴露到局域网，可以运行
`windows/deployment/start-lan-preview.bat`。脚本要求手动输入 `SHARE` 才会
绑定 `0.0.0.0`，并明确提示这是纯 HTTP 的可信局域网预览；`--lan-preview` 与
HTTPS 生产模式分离，不会代替正式 HTTPS，也不会自动修改防火墙规则。

## 正式 HTTPS

启动前先执行只读生产预检：

```powershell
.\.venv\Scripts\python.exe -m token_tracker preflight --production
```

1. 将域名 DNS 指向中心主机或 VPS。
2. 使用 Caddy/Nginx 终止 HTTPS，并仅反代到 `127.0.0.1:5000`。
3. `.env` 设置非占位、至少 32 字符的随机 `TOKEN_TRACKER_SECRET_KEY`、`TOKEN_TRACKER_SECURE_COOKIE=1` 和 provider HTTPS allowlist；`serve --production` 会在启动前校验该密钥。
4. 使用 Waitress 或 Windows 服务管理器启动 Python 进程，不使用 Flask development server。可直接使用项目内的启动包装器：
   ```powershell
   .\deployment\start-production.ps1 -BindAddress 127.0.0.1 -BindPort 5000
   ```
   该脚本先执行 `preflight --production`，失败时不会创建应用或监听端口；它不负责申请证书、修改防火墙或创建 Windows 服务。
5. 使用内置命令创建经过 SQLite 完整性校验的本地备份，并按组织策略测试恢复；不要把数据库或备份放进网站静态目录。
   ```powershell
   cd D:\Workplace\Agent_Workplace\ai-token-tracker\windows
   .\.venv\Scripts\python.exe -m token_tracker backup
   ```
   默认备份写入 `windows/data/backups/`。备份文件包含所有账户和日志，必须按敏感数据处理；命令不会上传云端或自动复制到其他机器。
   发布前可以使用 `.\.venv\Scripts\python.exe -m token_tracker verify-backup --path <backup.sqlite3>` 做只读验证；真正恢复必须在明确的 staging 目标上演练：
   ```powershell
   .\.venv\Scripts\python.exe -m token_tracker restore-backup `
     --path <backup.sqlite3> `
     --target <staging\token_tracker.sqlite3>
   ```
   恢复默认拒绝覆盖已有目标；只有确认目标文件可替换时才显式追加 `--overwrite`。命令先只读校验源备份，再写 sibling partial 文件，完成完整性/外键/schema 校验后原子发布。
6. 将 Android `trackerApiBaseUrl` 指向同一个 `https://host.example/api/v1`。

## Caddy 边缘配置

复制 `Caddyfile.example`，将 `tracker.example.com` 替换为已经指向中心主机的真实域名，
并在 Caddy 工作目录创建 ACL 受限的 `logs/` 目录。启动前先验证配置：

```powershell
.\deployment\preflight-edge.ps1 `
  -ConfigPath .\deployment\Caddyfile `
  -LogsDirectory .\deployment\logs
caddy run --config .\deployment\Caddyfile --adapter caddyfile
```

Caddy 负责 HTTPS 证书和边缘安全响应头，Waitress 只绑定 `127.0.0.1`；反向代理不能替代
应用自身的认证、RBAC、CSRF、限流和脱敏日志。上线后应从外部网络检查
`https://host.example/api/health`、`https://host.example/api/v1/ready`、登录、Android
`/api/v1/health` 和安全响应头；其中 health 是存活探针，ready 是 SQLite 就绪探针。
`preflight-edge.ps1` 是只读门禁：Caddy 未安装、配置无效或日志目录存在宽泛写权限时会失败，
不会自动安装软件、申请证书、修改 ACL、防火墙或启动服务。

## 回滚顺序

1. 在 Caddy/Windows 服务管理器层停止对外流量和 Waitress。
2. 保留当前数据库文件，使用 `verify-backup` 验证指定备份。
3. 用 `restore-backup` 恢复到隔离 staging 目标，检查完整性、外键和必要表。
4. 由负责人确认备份时间点和数据范围后，显式使用 `--overwrite` 替换已停止服务使用的目标文件。
5. 重新执行 production preflight，启动 `start-production.ps1`，再通过 HTTPS health 和登录检查。

回滚演练必须记录目标路径、操作者、时间、备份标识和验证结果；当前仓库只提供命令与边界，
不会自动复制或覆盖现有同学数据库。

## 未来托管

如果不希望个人电脑一直在线，可以把 `windows/` 部署到一台 Windows VPS 或迁移到 Linux/Go 服务；只要保留 `/api/v1` 和数据迁移契约，网页/Android 无需重写。当前项目不自动创建云资源、不上传同学数据，部署权限和域名由项目所有者明确控制。
