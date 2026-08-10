# AI Token Tracker

本项目是一个本地优先的 AI Token 用量追踪器：Windows 端负责中心服务与数据库，Android 端
通过同一套 `/api/v1` 契约登录、查看和写入数据。管理员可以查看成员汇总、用量明细、工作事件、
错误码和脱敏日志导出。

## 先体验

在 Windows 上直接双击根目录的 `start.bat`。它会进入 `windows/`，准备项目虚拟环境，启动本地
网页并自动打开浏览器；默认地址是 `http://127.0.0.1:5000`，若端口被占用会自动选择附近空闲端口。

首次进入网页后注册账户；如果要把账户设为管理员，在 `windows/` 目录执行：

```powershell
.\.venv\Scripts\python.exe -m token_tracker admin set-role --username your-name --role admin
```

## 目录

```text
ai-token-tracker/
├─ windows/   # Flask 服务、SQLite、CLI、Web UI、EXE 和部署配置
├─ android/   # Kotlin + Jetpack Compose 客户端
├─ start.bat  # 根目录一键体验入口
└─ README.md  # 本页总览
```

## 文档入口

- [Windows 服务与 CLI](windows/README.md)
- [Android 构建与联调](android/README.md)
- [正式 HTTPS 部署](windows/deployment/README.md)
- [Windows EXE 打包](windows/packaging/README.md)
- [企业级架构](windows/docs/enterprise-architecture.md)
- [交付任务与外部门禁](windows/tasks/todo.md)

## 重要边界

- 个人体验数据默认保存在 Windows 本地 `windows/data/`；同学共享时，所有账户进入同一台中心
  Windows 服务，不能把 SQLite 文件直接发给同学。
- 正式分享必须使用 HTTPS、真实长随机 Session Secret、备份/恢复策略和受控访问日志；纯 HTTP
  LAN 预览只适合可信网络。
- EXE、APK 和正式 Caddy/HTTPS 仍需在批准的外部工具链/部署主机完成，不会在未批准时自动下载、
  安装或伪造产物。
- 代码、脚本和端侧源码遵守单文件不超过 1000 行以及企业级作者头/接口注释约束。
