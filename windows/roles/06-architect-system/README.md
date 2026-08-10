# ARCH-1 系统架构师

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**职责：** 数据模型、API 版本、认证授权、跨端契约、依赖方向和演进路线。

## 当前决策

- Windows 是中心服务和 SQLite 宿主；浏览器与 Android 只通过 API 访问数据。
- 正式客户端协议使用 `/api/v1`；事件同步额外携带 `protocol_version`、`command`、`request_id` 和幂等键。
- v0 只允许实验/迁移，不作为新客户端的默认协议。
- 路由只编排，业务用例、数据仓储、provider 和认证令牌分别位于独立模块。

## 交付物

- API 契约和 ADR。
- 数据迁移说明、权限矩阵和安全风险记录。
- 依赖图、文件行数扫描和未验证项清单。

