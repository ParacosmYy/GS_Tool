# Windows EXE 打包与部署

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**用途：** 生成个人单机体验 EXE；团队共享仍使用中心 Windows 网站。

## 选择正确的产品形态

| 形态 | 适合谁 | 数据位置 | 推荐 |
|---|---|---|---|
| HTTPS 网站 | 你和同学共同使用 | 你的 Windows 主机/服务器 | 团队共享首选 |
| Windows EXE | 你个人离线体验 | 当前 Windows 用户的 LocalAppData | 个人体验首选 |
| Android APK | 手机端查看/上报 | 仍在中心服务端 | 与网站共用账号 |

不要把 EXE 分发给每位同学后再要求管理员看全部数据；每个 EXE 都会形成自己的本地数据库。团队数据必须通过同一个 HTTPS Windows 服务地址进入中心 SQLite/后续 PostgreSQL。

## 构建

在 `windows/` 目录执行：

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\packaging\toolchain-doctor.ps1
.\packaging\build.ps1
```

`toolchain-doctor.ps1` 是只读检查：它只验证项目虚拟环境、锁文件、入口资源和
PyInstaller 模块/启动器是否存在，不执行 Python 导入、不下载依赖、不修改 PATH、不清理 `dist/`/`build/`。缺少
批准的构建工具时会以 exit code `3` 报告 `PENDING`；不要把这个状态当成 EXE 已构建。

脚本会先按 `windows/requirements.lock` 和 `packaging/requirements-build.lock` 对齐运行时与构建依赖，生成 `windows/dist/AI-Token-Tracker/AI-Token-Tracker.exe`。首次启动会在 `%LOCALAPPDATA%\AITokenTracker\token_tracker.sqlite3` 创建持久化数据库，不会把数据写入 EXE 临时解包目录。PyInstaller 仍是独立的构建工具依赖，发布前必须在批准的构建环境中执行并审查版本。

构建成功后可以生成同学直接解压使用的 ZIP：

```powershell
.\packaging\package.ps1 -Version 0.1.0
```

输出为 `windows/release/AI-Token-Tracker-windows-x64-0.1.0.zip`，内含 onedir 运行目录、终端用户
`README.txt`、`RELEASE-MANIFEST.json` 和只读 `VERIFY-PACKAGE.ps1`。ZIP、EXE 和用户数据库都不提交 Git；
签名和正式发布渠道仍需部署负责人另行验收。

验证解压后的包（不会启动 EXE 或修改数据）：

```powershell
.\packaging\verify-package.ps1 -PackageDirectory .\release\AI-Token-Tracker-windows-x64-0.1.0 -ExpectedVersion 0.1.0
```

升级时先备份并停止旧 EXE，再把新包解压到新目录并通过 manifest/hash 校验；回滚时停止新 EXE，
重新启动上一份已验证包，保留 `%LOCALAPPDATA%\AITokenTracker` 用户数据目录。当前 checkout 已通过
隔离的真实 API 升级/回滚演练；演练入口如下：

```powershell
.\packaging\verify-upgrade-rollback.ps1 `
  -PreviousPackageDirectory .\.cache\package-verify-v12-final `
  -CurrentPackageDirectory .\.cache\exe-v13-package-verify-20260810 `
  -Port 5020
```

该入口只使用项目缓存目录，并通过 `POST /api/v1/records` 验证同一用户数据在升级和回滚后仍可读取。
Authenticode 签名和正式分发仍需要部署负责人验收，详细证据见 [`ADR-092`](../docs/decisions/ADR-092-exe-upgrade-rollback-evidence.md)。

构建前必须检查：

- `TOKEN_TRACKER_SECRET_KEY` 通过部署环境注入，不把真实密钥写进包。
- provider allowlist、HTTPS、备份和 Windows 防火墙策略已经确定。
- `dist/` 只作为构建产物，不提交 Git；源码体验入口仍是根目录 `start.bat`。
- 构建环境允许访问已批准的 Python 包缓存；脚本不会使用未锁定的 PyInstaller 版本。

当前 checkout 已按锁文件安装 PyInstaller 6.22.0，并完成 v13 `--onedir` EXE 构建、隔离启动及升级/回滚验收。
构建产物的 SHA-256、用户数据目录和已知未完成门禁见 [`ADR-091`](../docs/decisions/ADR-091-exe-v13-rebuild-evidence.md)。
这不等于正式分发完成：签名、中心 HTTPS 和真实部署数据恢复仍需单独验收。

## Authenticode 签名

正式发布必须先在批准的 Windows SDK/SignTool 环境中签名，再重新生成 ZIP：

```powershell
.\packaging\sign-build.ps1 `
  -CertificateThumbprint "<approved-code-signing-thumbprint>" `
  -TimestampUrl "https://<approved-timestamp-service>"
.\packaging\package.ps1 -Version 0.1.0
.\packaging\verify-signature.ps1 -PackageDirectory .\dist\AI-Token-Tracker `
  -ExpectedThumbprint "<approved-code-signing-thumbprint>"
```

证书私钥、timestamp 服务和 `signtool.exe` 不进入仓库或发布包。缺少任一项时签名门禁失败，不能用本地自签名证书替代正式发布证书；边界和官方依据见 [`ADR-093`](../docs/decisions/ADR-093-authenticode-signing-gate.md)。

## 团队分享

团队分享需要在 Windows 中心机运行：

```powershell
python -m token_tracker serve --host 127.0.0.1 --port 5000 --production
```

正式环境请放在 Caddy/Nginx HTTPS 反向代理后，并为同学提供代理网站地址；生产 Waitress 只绑定 loopback，不能直接把应用端口暴露到公网。生产 Windows 服务可调用 `../deployment/start-production.ps1`，它会用实际绑定地址执行 HTTPS 预检。
