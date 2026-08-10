# ADR-087：Windows EXE v12 重建与分发包证据

**作者：** AI Token Tracker Engineering Team
**维护者：** ARCH-2 / DEV-5
**状态：** Accepted
**日期：** 2026-08-10
**替代范围：** ADR-082 的旧版 EXE/ZIP 构建证据；构建边界和用户数据边界仍由 ADR-082 约束。

## 背景

Web 与 Android 已切换到 v12 品牌场景，但 ADR-082 记录的 EXE/ZIP 是 v11 资源构建结果。若不重建，根目录源码体验、Android 客户端和可分享 Windows 程序会呈现不同视觉版本，违反交付一致性。

## 决策

使用锁定的 PyInstaller 6.22.0 和现有 `packaging/build.ps1`、`packaging/package.ps1` 重建 onedir EXE，并让发布审计引用本次实际产物哈希。构建只把模板和静态资源打入 EXE，不改变认证、API、数据库 schema 或用户数据目录策略。

## 构建与隔离运行证据

在 2026-08-10 当前 checkout 执行：

```powershell
Set-Location windows
.\packaging\build.ps1 -Clean
.\packaging\package.ps1 -Version 0.1.0
```

- EXE：`windows/dist/AI-Token-Tracker/AI-Token-Tracker.exe`
- EXE 大小：`5,789,359` bytes
- EXE SHA-256：`4B8C52BC393CFD7F7D6D3530A6C028EA3949DC3C13BFF28AF3ECCEAD38651DCE`
- ZIP：`windows/release/AI-Token-Tracker-windows-x64-0.1.0.zip`
- ZIP 大小：`36,427,169` bytes
- ZIP SHA-256：`5A8E05660A692FB82F0F3EE9636D2D902B6A53CBECE597B0E705BB0FA23049BE`
- ZIP 清单包含 `AI-Token-Tracker.exe`、`README.txt` 和 `embedded-rust-engineer-bg-v12.png`。
- 使用隔离的项目缓存 `LOCALAPPDATA` 和端口 5019 启动 EXE：`GET /login`、`GET /api/v1/ready` 与 v12 PNG 均返回 200。
- 隔离数据库创建在 `LOCALAPPDATA\AITokenTracker\token_tracker.sqlite3`，EXE 发布目录没有生成 `data/token_tracker.sqlite3`。
- 验收结束后已停止本次 EXE，未触碰 5000/5011 既有服务。

## 未完成门禁

真实记录停止/重启持久化、升级/回滚、Windows Authenticode 签名、正式分发渠道和用户数据恢复仍未由本 ADR 虚构通过；这些门禁继续由最终验收矩阵跟踪。

## 回滚

将源码引用恢复到 v11 或使用 ADR-082 记录的旧版构建保存点即可回滚；不要通过 Git 删除用户 `%LOCALAPPDATA%\AITokenTracker\` 数据。
