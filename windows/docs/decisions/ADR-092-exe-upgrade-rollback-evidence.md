# ADR-092：Windows EXE 升级与回滚实证

**作者：** AI Token Tracker Engineering Team
**维护者：** BE / ARCH-2
**状态：** Accepted
**日期：** 2026-08-10
**前置：** ADR-089、ADR-091

## 目的

manifest 和 verifier 已经约束了包完整性，但还需要证明“替换包目录不会丢失用户数据”。本记录把上一版本、当前版本和回滚版本放进同一隔离流程，使用公开 `/api/v1` 读写边界验证数据库持久性。

## 可重复入口

从 `windows/` 执行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\packaging\verify-upgrade-rollback.ps1 `
  -PreviousPackageDirectory .\.cache\package-verify-v12-final `
  -CurrentPackageDirectory .\.cache\exe-v13-package-verify-20260810 `
  -Port 5020
```

脚本只启动两个显式包目录中的 EXE，使用新的 `windows/.cache` LocalAppData 隔离目录和 loopback 端口；不会删除包、用户数据或现有服务，也不会输出账户密码和 bearer token。

## 2026-08-10 运行证据

- v12 verifier 通过，EXE SHA-256：`1FE1EBE3223B934E51529584721669E3675808CD477CDBB3A576B1239FD96CA1`。
- v13 verifier 通过，EXE SHA-256：`54E79B836E7C736543E41788D887FE0AECCD316E0CEB64757C1CA9466E4A0E4F`。
- v12 EXE 启动后通过真实注册、登录和 `POST /api/v1/records` 写入 `record_id=1`，`222 + 333`。
- v13 EXE 使用同一隔离 LocalAppData 重启后读取到同一个 `record_id=1`，输入/输出仍为 `222 / 333`。
- 停止 v13 后重新启动 v12，回滚包再次读取同一记录，证明数据目录未随升级丢失。
- 过渡期间资源响应均为 `200`：v12 `2,007,338` bytes，v13 `1,741,675` bytes，回滚 v12 `2,007,338` bytes。
- 使用端口 `5020`，验证结束后监听数为 `0`；受保护的 5000/5011 服务未被触碰。
- 隔离数据目录：`windows/.cache/exe-upgrade-rollback-v13-20260810-r2/localappdata/AITokenTracker`。

## 结论与边界

当前 checkout 已有本地 EXE 升级/回滚和用户数据保留证据。Authenticode 签名、正式下载渠道、生产中心数据恢复和部署负责人签署仍不属于本次隔离演练，继续由最终验收矩阵跟踪。
