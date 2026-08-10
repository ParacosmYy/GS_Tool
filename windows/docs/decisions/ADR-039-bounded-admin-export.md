# ADR-039：管理员导出采用有界游标读取

## 状态

已接受（2026-08-10）。

## 背景

管理员 CSV 导出属于跨用户敏感数据出口。旧实现对 `usage_records`、`work_events` 和
`app_logs` 使用 `fetchall()`，再把全部行拼进无界 `StringIO`。数据增长时，导出请求的
内存峰值会随表大小线性增长，也没有稳定的 HTTP 超限语义。

## 决策

1. `admin_data.py` 只使用固定列投影和 SQLite 游标逐行消费，不把完整结果集载入列表。
2. 单次管理员导出最多 100,000 行、最多 16 MiB（包含 UTF-8 BOM）；任一边界超出就
   失败，不返回部分 CSV，也不静默截断。
3. `admin_service.py` 将数据层超限转换为稳定的 Application 异常，并记录
   `admin.export.rejected` 审计事件；`/api/v1/admin/export` 返回 `413 EXPORT_TOO_LARGE`。
4. 成功导出继续返回固定列、脱敏范围内的 UTF-8 BOM CSV，并记录 `admin.export` 审计事件。

## 取舍

- 16 MiB 和 100,000 行是当前 SQLite/Windows 本地部署的保守出口预算，不是永久产品上限；
  未来需要更大导出时，应增加按时间/用户/游标分页的契约，而不是移除边界。
- 返回值仍在单次请求内存中，但被明确限制；后续若需要更低峰值，可在不改变列契约的
  前提下改为真正的响应流式传输。
- 个人 `/api/export` 仍由既有个人查询边界维护；本 ADR 只覆盖管理员跨用户导出，避免
  在一个变更中同时改变两个客户端契约。

## 性能证据

- 基线结构检查：管理员导出路径存在 3 个 `fetchall()` 调用，输出缓冲没有行数/字节上限。
- 变更后结构检查：管理员导出路径改为游标迭代，`MAX_EXPORT_ROWS=100000`、
  `MAX_EXPORT_BYTES=16 MiB`，超限通过 `ExportTooLargeError` 传播到 413。
- 验证门禁：Python 编译、静态边界扫描、空 schema 导出响应和 API 错误 envelope 冒烟；
  不创建项目测试数据，不读取真实数据库内容。
