# ADR-097：LAN 与 HTTPS 部署预检证据边界

**作者：** AI Token Tracker Engineering Team  
**维护者：** BE / ARCH-2  
**状态：** Accepted；正式部署仍 pending  
**日期：** 2026-08-10  
**关联：** ADR-061、ADR-069、ADR-071、ADR-083、ADR-093

## 背景

项目需要让个人入口、可信局域网预览和正式 HTTPS 分享使用同一套可重复门禁。源码审计、
Android 工具链 pending、应用配置、Caddy 配置和日志目录属于不同责任边界，不能把示例
配置的预检结果描述成真实域名已经上线。

## 决策

继续使用 `release-doctor.ps1/.bat` 编排 `Local`、`LanPreview` 和 `Production`，并保留
三态结果：`0` 表示全部通过，`3` 表示仍有外部 pending，`2` 表示至少一项失败。部署
doctor 只读，不安装软件、不启动 Waitress/Caddy、不申请证书、不改防火墙或 ACL。

## 2026-08-10 预检证据

- `release-doctor.ps1 -Mode LanPreview`：源码审计 `pass=14 pending=1 fail=0`；LAN handoff
  通过，绑定目标为 `0.0.0.0:5000`，没有启动服务。Android SDK API 37/Build Tools
  因 license 未由用户交互确认而保持 pending；最终汇总为 `pass=3 pending=2 fail=0`。
- Production 预检使用进程级临时环境变量 `TOKEN_TRACKER_SECURE_COOKIE=1` 和示例
  `TOKEN_TRACKER_ALLOWED_BASE_URLS`，未写入 `.env`。`Caddyfile.example` 与隔离日志目录
  `.cache/caddy-preflight-v12/logs` 通过：应用 production preflight、Caddy `Valid configuration`、
  edge ACL/路径预检和 `127.0.0.1:5000` → Caddy handoff 均成功；最终汇总仍为
  `pass=3 pending=2 fail=0`，pending 原因仍只来自 Android SDK license。

## 证据边界与未关闭项

本 ADR 不证明真实域名、DNS、证书签发、Windows 防火墙、正式日志 ACL/轮转、外部
health/ready、真实 Provider 或备份责任人已经完成。正式上线前必须替换示例域名，使用
部署主机实际配置和受限日志目录，再执行 production preflight，并从外部网络完成 health、
ready、登录和安全响应头检查。

## 回滚

本次变更只有文档证据；删除本 ADR 及其引用即可回退，不改变运行时代码、配置、数据库或
部署主机状态。
