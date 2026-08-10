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
.\packaging\build.ps1
```

脚本会先按 `windows/requirements.lock` 和 `packaging/requirements-build.lock` 对齐运行时与构建依赖，生成 `windows/dist/AI-Token-Tracker/AI-Token-Tracker.exe`。首次启动会在 `%LOCALAPPDATA%\AITokenTracker\token_tracker.sqlite3` 创建持久化数据库，不会把数据写入 EXE 临时解包目录。PyInstaller 仍是独立的构建工具依赖，发布前必须在批准的构建环境中执行并审查版本。

构建前必须检查：

- `TOKEN_TRACKER_SECRET_KEY` 通过部署环境注入，不把真实密钥写进包。
- provider allowlist、HTTPS、备份和 Windows 防火墙策略已经确定。
- `dist/` 只作为构建产物，不提交 Git；源码体验入口仍是根目录 `start.bat`。
- 构建环境允许访问已批准的 Python 包缓存；脚本不会使用未锁定的 PyInstaller 版本。

当前 checkout 未安装 PyInstaller，因此只保留可复现构建脚本和锁文件，不能声称 EXE 已生成或验收通过。

## 团队分享

团队分享需要在 Windows 中心机运行：

```powershell
python -m token_tracker serve --host 0.0.0.0 --port 5000 --production
```

正式环境请放在 HTTPS 反向代理后，并为同学提供网站地址；不要直接把 Flask development server 暴露到公网。生产 Windows 服务可调用 `../deployment/start-production.ps1`，它会先执行 HTTPS 预检。
