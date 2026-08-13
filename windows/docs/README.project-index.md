# AI Token Tracker

本项目是一个本地优先的 AI Token 用量追踪器：Windows 端负责中心服务与数据库，Android 端
通过同一套 `/api/v1` 契约登录、查看和写入数据。管理员可以查看成员汇总、用量明细、工作事件、
错误码和脱敏日志导出。

## 先体验

在 Windows 上直接双击根目录的 `start.bat`。它会进入 `windows/`，准备项目虚拟环境，启动本地
网页并自动打开浏览器；默认地址是 `http://127.0.0.1:5000`，若端口被占用会自动选择附近空闲端口。
也可以使用已准备好的虚拟环境从根目录执行 `windows\.venv\Scripts\python.exe run.py`；根目录
`run.py` 只是稳定转发入口，不复制 Web 业务逻辑。
个人入口始终 fail-closed 绑定 `127.0.0.1`；不要通过 `TOKEN_TRACKER_HOST` 把本机账本
隐式暴露到局域网。需要分享时请使用文档中的显式 `--lan-preview` 或 HTTPS 中心部署路径。

首次进入网页后注册账户；如果要把账户设为管理员，在 `windows/` 目录执行：

```powershell
.\.venv\Scripts\python.exe -m token_tracker admin set-role --username your-name --role admin
```

## 自动采集外部客户端

网页里的“自动采集”会在模型检测和调用完成后读取上游真实 `usage` 并自动入账，不需要日常手动
填写 token 数。Kimi Code、OpenAI SDK 等独立客户端不能被网页偷偷观察；如果希望它们自动进入账本，
必须让客户端把 Base URL 指向项目提供的本地 Gateway：

```powershell
cd D:\Workplace\Agent_Workplace\ai-token-tracker\windows
python -m token_tracker ingest-token create --username your-name --label kimi-code --expires-days 90
# 将命令只显示一次的 ait_... 保存到 windows/.env；同时在 .env 设置：
# TOKEN_TRACKER_GATEWAY_PROVIDER_KEY=<你的 provider key>
# TOKEN_TRACKER_GATEWAY_INGEST_TOKEN=<上一步的 ait_...>
.\start-gateway.bat -AllowHttp
```

然后将兼容客户端的 Base URL 改为 `http://127.0.0.1:8787/v1`。`-AllowHttp` 仅适用于本机调试；
正式中心服务和跨设备使用必须改为 HTTPS。Gateway 会自动提取非流式 JSON 或流式 SSE 的真实 usage，
没有完整 usage 时明确标记未记录，不会猜测。完整配置、撤销 token 和生产边界见
[`windows/README.md`](windows/README.md) 的“如何自动接入”章节。

## 目录

```text
ai-token-tracker/
├─ windows/   # Flask 服务、SQLite、CLI、Web UI、EXE 和部署配置
├─ android/   # Kotlin + Jetpack Compose 客户端
├─ run.py     # 根目录 Python 体验入口，转发到 windows/run.py
├─ start.bat  # 根目录一键体验入口
└─ README.md  # 本页总览
```

## 文档入口

- [Windows 服务与 CLI](windows/README.md)
- [Android 构建与联调](android/README.md)
- [正式 HTTPS 部署](windows/deployment/README.md)
- [Windows EXE 打包](windows/packaging/README.md)
- [企业级架构](windows/docs/enterprise-architecture.md)
- [发布就绪审计](windows/docs/release-readiness.md)
- [交付任务与外部门禁](windows/tasks/todo.md)

## 重要边界

- 个人体验数据默认保存在 Windows 本地 `windows/data/`；同学共享时，所有账户进入同一台中心
  Windows 服务，不能把 SQLite 文件直接发给同学。
- 正式分享必须使用 HTTPS、真实长随机 Session Secret、备份/恢复策略和受控访问日志；纯 HTTP
  LAN 预览只适合可信网络。
- EXE、APK 和正式 Caddy/HTTPS 仍需在批准的外部工具链/部署主机完成，不会在未批准时自动下载、
  安装或伪造产物。
- 代码、脚本和端侧源码遵守单文件不超过 1000 行以及企业级作者头/接口注释约束。
