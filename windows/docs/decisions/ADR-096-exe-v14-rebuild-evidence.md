# ADR-096：Windows EXE v14 资源一致性与回滚证据

**作者：** AI Token Tracker Engineering Team
**维护者：** FE / BE / ARCH-2
**状态：** Accepted
**日期：** 2026-08-10
**关联：** ADR-095、ADR-091、ADR-092、ADR-093

## 背景

ADR-095 将 Web 与 Android 的默认品牌背景切换为 v14。根目录个人入口和可分享 Windows 包必须重新构建，否则源码、EXE 与 Android 会出现视觉版本分叉。本记录只证明当前 checkout 的 v14 包一致性和隔离运行，不替代正式签名、正式分发或真实部署验收。

## 构建与包验证

- 使用项目锁定的 PyInstaller `6.22.0` 运行 `windows/packaging/build.ps1 -Clean`。
- 使用 `windows/packaging/package.ps1 -Version 0.1.0` 生成 onedir ZIP。
- EXE SHA-256：`5F6A32659D016419ED7B43F98A1E6B94AF3FE98346890915C4CEA8683EDE20A6`。
- ZIP SHA-256：`20FDC0168254B05B4D5984E39C7DA9315D76C71ACD916685D9780010C18E6713`。
- `verify-package.ps1 -ExpectedVersion 0.1.0` 通过；包内用户数据边界仍为 `%LOCALAPPDATA%\AITokenTracker`，包内没有 `data/`。
- 包内 v14 资源大小 `1,660,273` bytes，SHA-256 与 ADR-095 的 Web/Android 资产一致。

## 隔离运行证据

使用项目缓存 LocalAppData 和端口 `5019` 启动当前 v14 EXE，未触碰受保护的 5000/5011 服务：

- `GET /api/v1/ready`：`200`。
- `GET /login`：`200`。
- `GET /static/assets/embedded-rust-engineer-bg-v14.png`：`200`，`1,660,273` bytes。
- 数据库创建在隔离 `%LOCALAPPDATA%\AITokenTracker`，没有写入 EXE 目录 `data/`。
- smoke 完成后只停止本轮精确匹配路径的 EXE PID，端口 `5019` 已释放。

## 升级与回滚证据

使用历史 v13 包与当前 v14 包执行 `verify-upgrade-rollback.ps1`，端口 `5023`，运行目录为项目缓存：

- 流程：`v13→v14→v13`。
- v13 与 v14 包 verifier 均通过。
- 通过真实 `/api/v1/records` 写入 `record_id=1`，输入/输出 `222 / 333`。
- v14 启动后读取同一记录；回滚 v13 后再次读取同一记录，数据未丢失。
- 资源响应：v13 `1,741,675` bytes，v14 `1,660,273` bytes，回滚 v13 `1,741,675` bytes。
- 隔离数据目录：`windows/.cache/exe-upgrade-rollback-v14-20260810/localappdata/AITokenTracker`。

## 未关闭门禁

本记录不表示 EXE 已签名。批准的 Authenticode 证书、SignTool、HTTPS 时间戳服务和正式分发仍由 ADR-093 与最终验收矩阵单独门禁；Android SDK license、APK 构建、真实设备和正式 HTTPS 部署也不由本记录替代。
