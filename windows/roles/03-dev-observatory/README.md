# DEV-2 Observatory / Analysis

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**职责：** 仪表盘首屏、总量 signal、趋势图、模型占比和数据状态。

## 硬约束

- 只消费 JSON API，不读取 SQLite，不在浏览器猜测 token。
- 图表必须能表达 loading、空数据、错误和 reduced-motion 状态。
- 总量、输入、输出和模型维度的字段含义以 `windows/docs/api-contract.md` 为准。
- 任何公共 API 变更先更新契约，再改图表映射。

