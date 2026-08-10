# ADR-091：Windows EXE v13 资源一致性证据

**作者：** AI Token Tracker Engineering Team
**维护者：** FE / BE / ARCH-2
**状态：** Accepted
**日期：** 2026-08-10
**前置：** ADR-087、ADR-089、ADR-090

## 目的

ADR-090 将 Web 与 Android 默认背景切换到 v13。若只修改源码而不重建可分享程序，根目录体验、网站、Android 和 EXE 会产生视觉版本分叉。本记录保存本次锁定构建和隔离启动证据。

## 构建与包

- 构建入口：`windows/packaging/build.ps1`，使用锁定 PyInstaller `6.22.0` 和 onedir 模式。
- 打包入口：`windows/packaging/package.ps1 -Version 0.1.0`。
- EXE：`windows/dist/AI-Token-Tracker/AI-Token-Tracker.exe`。
- EXE SHA-256：`54E79B836E7C736543E41788D887FE0AECCD316E0CEB64757C1CA9466E4A0E4F`。
- ZIP：`windows/release/AI-Token-Tracker-windows-x64-0.1.0.zip`。
- ZIP SHA-256：`2D03BFE45E97CB23E17267998580A00BB4565A9A03A55485E47368E2CF2B7EEC`。
- 包内 `RELEASE-MANIFEST.json` 与 `VERIFY-PACKAGE.ps1` 已按 ADR-089 的数据目录边界生成。
- 解压后只读 verifier 通过，包版本 `0.1.0`、平台 `windows-x64`、EXE 哈希和 `%LOCALAPPDATA%\AITokenTracker` 边界均匹配。

## v13 运行证据

在未触碰受保护的 5000/5011 服务前提下，使用隔离 `windows/.cache/exe-v13-smoke-localappdata-20260810` 和端口 `5019` 启动本次 EXE：

- `GET /api/v1/ready`：`200`。
- `GET /login`：`200`。
- `GET /static/assets/embedded-rust-engineer-bg-v13.png`：`200`，`1,741,675` bytes。
- 隔离用户数据库已创建在 `%LOCALAPPDATA%\AITokenTracker\token_tracker.sqlite3`。
- EXE 解压目录未创建 `data/token_tracker.sqlite3`。
- 验证后已停止本次 EXE，端口 `5019` 监听数为 `0`。

## 未覆盖范围

本记录证明当前 checkout 的 v13 EXE 资源一致性与隔离启动，不证明 Authenticode 签名、真实升级/回滚、正式下载渠道、Android 构建或生产 HTTPS 部署。上述事项继续由最终验收矩阵单独门禁。
