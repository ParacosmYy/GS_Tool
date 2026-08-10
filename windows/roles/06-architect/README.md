# 架构师交付包

**作者：** AI Token Tracker Engineering Team  
**职责：** 维护系统边界、公共契约、ADR、集成顺序、风险清单和最终交付闸门。

## 输入

- 用户需求和 `AGENTS.md`
- 历史角色交付包
- `docs/architecture.md`、`docs/api-contract.md`、`docs/decisions/`

## 输出

- API、动画、设计 token 和数据模型契约；
- 变更对应的 ADR；
- 依赖图、回滚边界、验证证据和未完成风险；
- 最终 root launcher 和文档交付检查。

## 禁止

- 为了“看起来独立”复制一份真实业务源代码；
- 让模板直接连接数据库；
- 允许角色绕过契约直接修改共享边界；
- 在没有证据时宣称生产安全、合规或完整自动读取外部进程。
