# ADR-085：源代码质量闸门与 CI 编排

作者：AI Token Tracker Engineering Team
维护者：ARCH / Project Owner
状态：Accepted
日期：2026-08-10
作用：固化提交级源码质量检查，避免本地门禁只靠人工记忆。

## 背景

项目已经有 token_tracker audit 和 release-doctor，但缺少一个可在每次提交/合并请求执行的 CI
入口。生产工具链、Android license、Caddy、Provider Key 和真实设备仍属于外部门禁，不能在 CI 中
用模拟状态代替；源码质量和契约一致性则应在无凭据、无数据库副作用的环境中自动执行。

## 决策

1. windows/ci/quality-gate.ps1 作为唯一源码质量编排入口，执行 git diff --check、Python
   compileall、项目 token_tracker audit --json、CLI help 和仓库 PowerShell AST 解析。
2. 质量闸门只允许 audit 的 fail=0 通过；pending 继续输出但不被伪装为 pass。Android/Caddy/真实
   Provider/设备状态由 release-doctor 和最终验收矩阵继续管理。
3. .github/workflows/quality-gate.yml 在 Windows runner 上安装两个锁文件并调用项目脚本；workflow
   只读 checkout，权限限定为 contents: read，不上传数据库、密钥、日志或构建产物。
4. CI 不创建测试专用资产，不运行默认单元测试，不启动服务，不申请证书，不安装 Android SDK license。

## 后果

- 新提交会自动暴露行数、作者头、契约引用、Python/PowerShell 解析和静态 UI/部署边界回归。
- 外部工具缺失不会被 CI 静默吞掉；它们保持 pending，并在发布负责人准备环境后由 release doctor 复核。
- GitHub Actions 使用官方 checkout/setup-python action 的主版本入口；如组织需要 SHA pin，应在部署
  策略确定后另立依赖更新切片，不把未核对的 hash 写入项目。

## 验证

- 本机已通过 PowerShell AST 解析、compileall、token_tracker audit --json 和 CLI help。
- CI workflow 尚未在远程仓库执行，因为当前 checkout 没有配置 remote；不能把本地 YAML 存在当成远程
  CI 已通过证据。
