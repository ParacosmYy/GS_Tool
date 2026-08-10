# ADR-016: 收敛用量与日志读取的 Application 边界

## Status

Accepted

## Date

2026-08-10

## Context

项目已经采用 `Presentation → Application → Infrastructure` 的分层约束，但
`web.py`、`api_v1.py` 仍直接调用部分 `db` 查询，`api_v1.py` 的日志分页还在控制器
内嵌 SQL。这样会让网页、Android 和未来 Vue/Go 迁移重复理解数据库字段，也会让统计
分页、时间范围和安全投影在不同入口产生漂移。

## Decision

本阶段按垂直切片收敛四类读取用例：

- `services.py` 暴露个人用量汇总、记录分页和 CSV 导出的应用函数；
- `events.py` 暴露个人诊断日志分页用例；
- `web.py` 和 `api_v1.py` 只保留请求解析、认证装饰器、用例调用和稳定响应映射；
- `db.py` 继续作为 SQLite 基础设施，暂不改变公共响应字段和 SQL schema。

应用函数负责调用统一的范围解析、分页上限和用户边界；路由不能复制这些规则。

## Alternatives considered

### 在每个路由里继续直接调用 `db`

实现成本最低，但控制器知道基础设施细节，新增 Vue/Go 或第二个客户端时会复制业务规则；
拒绝。

### 立即创建复杂的 Repository/Unit-of-Work 框架

可以表达更强的抽象，但当前项目只有 SQLite 和有限用例，会增加类型、生命周期和事务
概念而没有实质收益；拒绝，等出现第二种基础设施或事务需求再引入。

### 重写成 Go/Vue 后再治理边界

会把当前缺口整体搬到新技术栈，并增加部署风险；拒绝，先在可运行基线中冻结边界。

## Consequences

- 用量统计、分页和导出规则只有一个应用入口，API 行为保持兼容。
- 控制器更容易审查，未来可替换数据库而不改 HTTP 路由。
- 当前 `services.py`、`events.py` 仍直接依赖 SQLite，这是应用到基础设施的明确单向依赖；
  当出现 PostgreSQL 或第二种存储时再提取显式 repository port。
- 认证用户查询和管理员读模型仍是后续切片，不在本 ADR 中伪装成已完成。

## Verification

- Python 编译检查和所有 JavaScript 语法检查通过。
- `GET /api/summary`、`GET /api/v1/me/summary`、分页记录、CSV 和日志接口的响应字段
  与 `windows/docs/api-contract.md` 保持一致。
- 通过静态检索确认控制器不再包含日志分页 SQL；文件规模仍小于 1000 行。
