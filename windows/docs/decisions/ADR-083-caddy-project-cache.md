# ADR-083：Caddy 项目缓存与显式 HTTPS 边缘启动

**作者：** AI Token Tracker Engineering Team  
**维护者：** ARCH-2 / DEV-7  
**状态：** Accepted  
**日期：** 2026-08-10  
**作用：** 固化 Windows 分享部署的 Caddy 工具链来源、缓存边界和启动责任。

## 背景

团队分享需要 Caddy 终止 HTTPS，再反代到只监听 loopback 的 Waitress。仅提供
`Caddyfile.example` 会让部署者自行寻找版本，降低复现性；把 Caddy 静默安装到系统或自动改防火墙、
证书和 ACL 又会扩大项目脚本权限边界。

## 决策

1. 以 Caddy v2.11.4 Windows amd64 release 为项目默认工具链。归档 URL、文件名和官方
   `caddy_2.11.4_checksums.txt` 中的 SHA-512 固定在 `deployment/provision-caddy.ps1`。
2. `provision-caddy.ps1` 只把归档和 `caddy.exe` 写入被忽略的
   `windows/.cache/caddy/2.11.4/`；使用 Windows 自带 `curl.exe` 下载、失败重试并在解压前校验哈希，
   默认传输超时为 600 秒，也可通过 `-DownloadTimeoutSeconds` 显式覆盖。
   它不修改全局 PATH，不注册服务，不改防火墙/ACL，不申请证书，不启动进程。
   这条 project-local caddy-project-cache 是工具链边界，不是生产数据目录。
3. `preflight-edge.ps1` 支持显式 `-CaddyPath`，否则优先系统 `caddy`，再回退到项目缓存；它只验证
   配置和日志目录 ACL，并在 Caddyfile 所在目录执行配置校验，避免相对日志路径污染调用者目录。
   ACL 写权限判定使用显式写入位，不把包含只读位的 `FullControl` 直接并入位掩码，避免把
   `ReadAndExecute` 误判为可写，同时仍能识别真正的宽泛 `FullControl`。
   `start-edge.ps1` 必须先通过该预检，再以前台进程启动 Caddy，方便由用户或
   已批准的 Windows 进程监督器托管。
4. 域名 DNS、证书、日志目录 ACL、防火墙和外部 health/ready 检查仍由部署负责人负责；项目缓存可用
   不等于 HTTPS 分享已经上线。

## 来源与完整性

| 组件 | 来源 | 完整性 | 项目路径 |
| --- | --- | --- | --- |
| Caddy v2.11.4 Windows amd64 | `https://github.com/caddyserver/caddy/releases/tag/v2.11.4` | `caddy_2.11.4_checksums.txt` 中的 SHA-512：`CD5CCFD86A4B40732CF715890D0DCA5BF3F63ADEFEC5A7914DE85ADF240C60CE7E5D2791631B88EF9758E46B23BB1730E020B9C5D696889740B284FFD4788E35` | `.cache/caddy/2.11.4/` |

## 验证与边界

- `provision-caddy.ps1`、`start-edge.ps1` 和 `preflight-edge.ps1` 已通过 PowerShell AST 解析；源码审计
  已接入所需文件、作者头和契约引用。
- 当前 checkout 已下载并校验 Caddy v2.11.4：`caddy version` 返回 `v2.11.4`，归档 SHA-512
  与固定值一致，`Caddyfile.example` 的 `caddy validate` 返回 `Valid configuration`。
  隔离配置与日志目录执行 `preflight-edge.ps1` 返回 `Edge preflight passed`，且调用者根目录没有
  生成相对路径日志文件。
  这只证明项目工具链和示例配置可用，不代表真实域名、DNS、证书、ACL 或外部 HTTPS 已上线。
- 真实域名、受限日志 ACL、证书签发和外部访问检查仍属于 R-11 部署门禁。

## 回滚

停止 Caddy 后，源码可以回滚到没有项目缓存入口的版本；缓存目录是忽略状态，清理前应由部署负责人
确认不再被运行进程使用。不要通过 Git 回滚删除真实证书、日志或用户数据库。
