# ADR-089：Windows EXE 版本清单与升级回滚边界

**作者：** AI Token Tracker Engineering Team
**维护者：** ARCH-2 / DEV-5
**状态：** Accepted
**日期：** 2026-08-10
**范围：** Windows x64 onedir ZIP 的版本、完整性和用户数据边界。

## 背景

EXE 运行时数据位于 %LOCALAPPDATA%\AITokenTracker，而发布包位于可替换的解压目录。
只有 README 和外部哈希时，用户升级前无法在本地确认包是否完整，也容易误删用户数据库。

## 决策

packaging/package.ps1 为每个 ZIP 写入 RELEASE-MANIFEST.json，锁定 manifest 版本、产品版本、
平台、EXE SHA-256、用户数据目录和回滚策略；同时随包提供只读 VERIFY-PACKAGE.ps1。
验证器拒绝绝对路径、包目录外的 EXE、EXE 哈希不匹配和包内 data/ 目录。

升级顺序固定为：备份并停止旧 EXE → 将新 ZIP 解压到新目录 → 运行 verifier → 启动新 EXE。
回滚顺序固定为：停止新 EXE → 保留 %LOCALAPPDATA%\AITokenTracker → 启动上一份已验证包；
数据库恢复必须使用项目的显式 staging/restore 命令，不直接替换运行中的 SQLite 文件。

## 当前包证据

- 版本：0.1.0，平台：windows-x64。
- EXE SHA-256：1FE1EBE3223B934E51529584721669E3675808CD477CDBB3A576B1239FD96CA1。
- ZIP SHA-256：1DB2C1C603E5E4011C835F5094AFA06905E95125EB2B7E986798BB5BC543ECCC。
- 从 ZIP 解压后执行 VERIFY-PACKAGE.ps1 -ExpectedVersion 0.1.0 返回 Package verified。

## 未完成门禁

Authenticode 签名、正式发布渠道、真实升级/回滚演练和生产数据恢复仍需部署负责人验收。
PyInstaller 构建当前未声称可复现字节级哈希；每次构建都必须以新 manifest 和新哈希为准。

## 回滚

删除或替换发布包前必须确认目标目录不被进程使用。不得删除 %LOCALAPPDATA%\AITokenTracker，
不得把用户数据库打入 ZIP，也不得通过 Git 回滚真实用户数据。
