# ADR-082：Windows EXE 构建与用户数据落盘证据

**作者：** AI Token Tracker Engineering Team  
**维护者：** ARCH-2 / DEV-5  
**状态：** Accepted  
**日期：** 2026-08-10  
**作用：** 固化个人 Windows EXE 的可复现构建、冻结资源和用户数据目录验收边界。

## 背景

项目需要保留一个同学可以直接启动的 Windows 个人体验入口，同时不能把 SQLite 数据写进
PyInstaller 的解包目录或 EXE 发布目录。团队共享仍然使用中心 Windows 服务；EXE 不承担多用户
中心数据库角色。

首次冻结运行验证发现，入口模块在 `main()` 之前导入配置，项目 `.env` 的相对
`TOKEN_TRACKER_DB=data/token_tracker.sqlite3` 会先于冻结存储策略生效。这样会使 onedir EXE 在
当前工作目录创建数据目录，违反用户数据边界。修复方式是把应用组合导入放到
`configure_frozen_storage()` 之后，让冻结运行时先设置用户目录环境变量，再加载 dotenv 配置。

## 决策

1. 使用项目虚拟环境中的 PyInstaller 6.22.0 和 `packaging/requirements-build.lock`；构建脚本使用
   `--onedir`，将模板、静态资源显式收集到 `token_tracker/templates` 和 `token_tracker/static`。
2. `packaging/build.ps1` 使用 `--specpath windows/build`，生成的 spec、build 和 dist 都属于忽略的
   构建状态，不进入 Git。构建脚本的输出和错误文本使用 Windows PowerShell 5 可稳定解析的 ASCII，
   避免旧版控制台对无 BOM UTF-8 脚本的误解析。
   `packaging/package.ps1` 再把 onedir 目录打成带终端用户说明的 ZIP，发布目录同样属于忽略状态。
3. 冻结 EXE 启动前由 `windows/run.py` 创建
   `%LOCALAPPDATA%\AITokenTracker\`，并将数据库默认指向该目录；源码运行仍使用项目 `data/`。
   用户显式注入的 `TOKEN_TRACKER_DB` 仍可用于批准的运维/隔离场景。
4. 本 ADR 只把“构建、静态资源加载、登录页启动和用户目录建库”作为已完成证据；升级、回滚、
   Windows 签名和正式分发仍是独立门禁，不能因本地 EXE 构建成功而宣称最终交付完成。

## 构建与运行证据

在 2026-08-10 的当前 checkout 中执行：

```powershell
.\packaging\toolchain-doctor.ps1
.\packaging\build.ps1 -Clean
```

实际项目路径应从 `windows/` 目录执行，完整命令为：

```powershell
Set-Location windows
.\packaging\toolchain-doctor.ps1
.\packaging\build.ps1 -Clean
```

结果：

- PyInstaller 6.22.0 可用，`windows/dist/AI-Token-Tracker/AI-Token-Tracker.exe` 生成成功。
- EXE 大小为 5,789,361 bytes；SHA-256 为
  `875BA5A25DC91AB3D6F063191E11041D88B1808B4D9DB802527876A5594A5EFF`。
- 在隔离端口 5019、无浏览器模式启动后，`GET /login` 返回 HTTP 200。
- 同一隔离运行创建数据库
  `%LOCALAPPDATA%\AITokenTracker\token_tracker.sqlite3`，文件大小为 110,592 bytes。
- EXE 发布目录没有产生 `data/token_tracker.sqlite3`；测试进程停止后 5000/5011 原有服务未受影响。
- PyInstaller warning 文件中未发现应用顶层模块缺失；其中 `pwd`、`grp`、`fcntl` 等是跨平台依赖的
  可选模块提示，不构成 Windows 运行失败证据。
- `packaging/package.ps1 -Version 0.1.0` 已生成
  `windows/release/AI-Token-Tracker-windows-x64-0.1.0.zip`；大小为 34,432,157 bytes，SHA-256 为
  `78973D45A08206062F9A99EDC9004004BB8C630B717EA6ECFD417A95FEFFE0C6`，ZIP 清单包含 `README.txt`。

## 未完成门禁

- 尚未做带真实用户记录的停止/重启持久化回归。
- 尚未生成 Windows Authenticode 签名、安装包或正式发布渠道。
- 尚未完成升级、回滚、备份恢复和中心 HTTPS 部署验收。

## 回滚

源码回滚时恢复 `windows/run.py` 的导入顺序和 `windows/packaging/build.ps1` 的对应变更即可；已生成的
`windows/build/`、`windows/dist/` 和用户 `%LOCALAPPDATA%\AITokenTracker\` 数据属于本机状态，不能通过
Git 回滚删除。需要清理本机数据时必须由用户明确执行并先完成备份。
