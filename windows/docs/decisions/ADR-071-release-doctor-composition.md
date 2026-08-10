# ADR-071：统一只读交付预检编排

**作者：** AI Token Tracker Engineering Team  
**状态：** Accepted  
**日期：** 2026-08-10

## 背景

项目已经分别拥有源码发布审计、EXE 工具链 doctor、Android 工具链 doctor 和分享部署预检。
如果交付人员需要记住多个入口，容易漏跑检查或把工具链 `pending` 误解为通过。企业级交付需要一个
稳定、可重复、可脚本化的编排入口，同时必须保持“预检不改变主机状态”的边界。

## 决策

1. 新增 `windows/release-doctor.ps1`，按 `Local`、`LanPreview`、`Production` 三种模式编排既有只读检查。
2. `Local` 执行源码审计、本机 loopback preflight、EXE doctor 和 Android doctor；`LanPreview` 额外执行
   `share-doctor.ps1 -Mode LanPreview`；`Production` 必须显式提供 Caddyfile 和受限日志目录后才执行生产预检。
3. 汇总退出码保持脚本契约：`0` 全部通过，`3` 存在外部工具链 pending，`2` 存在失败；脚本会继续执行剩余检查，
   不因一个缺失工具隐藏其他结果。
4. 编排器不安装软件、不启动 Flask/Waitress/Caddy、不创建数据库或日志目录、不修改防火墙/ACL、不申请证书，
   不接收 Provider Key 或其他秘密参数。
5. `release-doctor.bat` 只负责稳定调用 PowerShell 入口；业务规则集中在 `.ps1`，避免双份实现漂移。

## 验收边界

- 通过：脚本解析、Windows PowerShell 5.1 `.bat` 包装器、Local/LAN/Production 模式输出完整源码/部署/工具链结果、
  退出码可脚本化、1000 行门禁、敏感参数不进入命令行；编排脚本保持 ASCII，避免无 BOM 的 Windows PowerShell 编码歧义。
- 待完成：具体机器上的 JDK/Android SDK、PyInstaller、Caddy、真实 Provider Key 和正式部署负责人验收；编排器
  只能准确报告这些外部状态，不能替代安装或实际联调。

## 回滚

删除或停止调用 `release-doctor.ps1/.bat` 即可恢复到各独立 doctor 入口；不删除既有单项预检器和其部署契约。
