# ADR-019: 分离 SQLite Schema 生命周期与查询实现

## Status

Accepted

## Date

2026-08-10

## Context

`db.py` 同时包含连接、事务、DDL、兼容迁移、用户查询、token 记录、统计和 CSV 投影，
文件已经接近项目 800 行拆分警戒线。DDL 与查询变化频率和审查责任不同，混在一起会
增加 schema 迁移误触查询逻辑的风险。

## Decision

新增 `schema.py` 作为 SQLite schema 生命周期边界：

- `ensure_schema(connection)` 负责表、索引和当前兼容迁移；
- `db.py` 继续负责数据库路径、连接、事务和历史查询函数；
- 迁移保持加法式，不重写历史记录；
- `db.init_db` 通过显式模块接口调用 schema，不改变调用方。

## Consequences

- DDL、迁移、连接和业务读模型可以独立审查，`db.py` 更容易继续按职责拆分。
- 当前仍使用 SQLite 和内置迁移，没有引入迁移框架；出现版本化 schema、多数据库或回滚
  演练需求时再建立专门 migration manifest。
- `schema.py` 只依赖 `sqlite3`，不得导入 Flask、应用服务或 provider。

## Verification

- 现有数据库执行 `init_db` 后表和索引保持可用。
- Python compileall、health/login smoke check、用量只读 smoke check 和文件规模扫描通过。
