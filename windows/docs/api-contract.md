# Web API 契约

**作者：** AI Token Tracker Engineering Team  
**状态：** Accepted；`/api/v1` 为网页、Android 和未来采集器的正式跨端契约。

所有 `/api/*` 受保护接口都需要已登录会话。修改数据的 `POST` 请求还需要 `X-CSRF-Token` 请求头；页面从 `<meta name="csrf-token">` 读取令牌。

实现边界：控制器只负责请求解析、认证装饰器和稳定响应映射；个人汇总、记录分页、
CSV 导出和日志分页由 Application 用例统一组合，时间范围、分页上限和用户隔离不能
由网页或 Android 控制器重复实现。具体决策见 ADR-016。

## 跨端版本与认证

`/api/*` 是当前网页兼容接口，继续保留；新客户端使用 `/api/v1/*`。网页可通过安全会话访问 v1，Android 使用 `Authorization: Bearer <access_token>`。v1 登录和刷新接口不需要 CSRF，但必须限流；浏览器会话写请求仍必须带 CSRF。

事件同步可使用以下命令信封。普通 REST 查询不需要额外包装：

```json
{
  "protocol_version": 1,
  "command": "work_event.create",
  "request_id": "client-generated-uuid",
  "idempotency_key": "client-generated-unique-key",
  "payload": {}
}
```

服务端只接受白名单命令和当前版本 `1`；同一用户重复提交相同幂等键必须返回第一次成功结果，不得重复写入。信封和 payload 不得携带密码、API Key、原始 prompt 或完整 provider 响应。

### `POST /api/v1/auth/login`

请求：`{"username":"alice","password":"至少 8 个字符"}`。

成功返回短期 `access_token`、较长期 `refresh_token`、`expires_in` 和不含密码哈希的 `user`。令牌只通过 HTTPS 传输；Android 应使用系统安全存储，不写普通日志。

凭据验证由认证 Application 边界完成；控制器不读取 `password_hash`，不存在用户和密码
错误对外使用同一 `INVALID_CREDENTIALS` 语义。

### `POST /api/v1/auth/refresh`

请求：`{"refresh_token":"..."}`。刷新令牌轮换出新的 access/refresh pair，旧 refresh token 立即失效。

### `POST /api/v1/auth/logout`

需要 bearer access token，撤销当前令牌。服务端仍允许管理员从数据库/运维侧撤销全部令牌。

### `GET /api/v1/me/summary?period=day`

需要登录，返回与 `/api/summary` 相同语义的个人汇总；响应不会跨用户聚合。

### `POST /api/v1/events/work`

需要登录。请求 payload 可直接提交工作事件，或放在上面的命令信封 `payload` 中：

```json
{
  "direction": "coding",
  "outcome": "success",
  "duration_ms": 420000,
  "efficiency_score": 86,
  "result_code": "OK",
  "error_code": null,
  "project": "token-tracker",
  "task_type": "feature",
  "note": "完成仪表盘联调"
}
```

`direction`、`outcome`、代码值和长度均由服务端白名单校验；事件不接受原始 prompt、密钥或任意 HTML。`efficiency_score` 为可选的 0-100 主观/规则评分，不代表生产绩效结论。

### `POST /api/v1/logs`

需要登录。接受 `level`、`event_type`、`message`、`error_code`、`request_id` 和有限 metadata；服务端会截断并脱敏。日志用于故障诊断，不等同于审计事件。

### 分页列表响应

`GET /api/v1/events/work` 与 `GET /api/v1/logs` 支持 `limit`、`offset` 查询参数，服务端会将 `limit` 限制在安全上限内，并返回兼容旧字段的列表和分页元数据：

```json
{
  "events": [],
  "pagination": { "limit": 50, "offset": 0, "total": 0 }
}
```

日志接口将列表字段命名为 `logs`；`total` 是当前用户可见资源总数。客户端不得根据当前页数量推断总量。

### 管理员接口

`admin` 角色才可访问：

- `GET /api/v1/admin/overview`：团队 token 总量、用户数、事件/错误计数和趋势。
- `GET /api/v1/admin/users`：用户列表及每人聚合，不返回密码哈希、令牌或 API Key。
- `GET /api/v1/admin/users/<user_id>/records`：查看指定用户的用量/事件分页明细。
- `GET /api/v1/admin/export?kind=usage|events|logs`：导出授权范围内的脱敏 CSV，并写审计事件。
  结果固定最多 100,000 行、16 MiB；超出时不返回部分文件，返回 `413 EXPORT_TOO_LARGE`，
  并写入拒绝审计事件。

管理员页面访问、用户明细查看、导出和角色变更都必须落入 `audit_events`。

### 运行状态接口

- `GET /api/v1/health`：进程存活探针，只证明 HTTP 路由可达，不读取业务表。
- `GET /api/v1/ready`：SQLite 核心 schema 就绪探针；成功返回 `status=ready`，失败返回
  `503 SERVICE_NOT_READY`，不暴露数据库路径、表名或内部异常。

管理员只读移动端使用以下稳定字段；服务端仍是最终授权者，客户端不能仅凭本地 `role` 绕过检查：

```json
// GET /api/v1/admin/overview
{
  "users": 3,
  "usage": { "calls": 12, "input_tokens": 1000, "output_tokens": 400, "total_tokens": 1400 },
  "work_events": { "total": 8, "success": 7, "failure": 1 },
  "logs": 4,
  "by_model": [],
  "trend": []
}
```

```json
// GET /api/v1/admin/users?limit=50&offset=0
{
  "items": [{
    "id": 2, "username": "alice", "role": "user", "created_at": "2026-08-10 12:00:00",
    "calls": 4, "input_tokens": 300, "output_tokens": 100, "total_tokens": 400,
    "work_events": 3, "logs": 1
  }],
  "limit": 50,
  "offset": 0,
  "total": 1
}
```

```json
// GET /api/v1/admin/users/2/records?limit=100
{
  "user": { "id": 2, "username": "alice", "role": "user", "created_at": "2026-08-10 12:00:00" },
  "records": [],
  "events": [],
  "logs": []
}
```

成员明细由服务端固定列和安全上限控制；Android 只展示脱敏投影，不缓存管理员导出文件。

当管理员导出超过安全边界时，响应仍使用统一错误 envelope：

```json
{
  "error": {
    "code": "EXPORT_TOO_LARGE",
    "message": "导出结果超过安全边界，请缩小范围或分批导出",
    "details": { "max_rows": 100000, "max_bytes": 16777216 }
  },
  "request_id": "8c6d3a4b"
}
```

## 通用错误

```json
{
  "error": {
    "code": "VALIDATION_ERROR",
    "message": "面向用户的简短错误说明",
    "details": {}
  },
  "request_id": "8c6d3a4b"
}
```

状态码约定：`400` 输入/CSRF 无效、`401` 未登录、`403` 无权、`404` 资源不存在、`405` 方法不支持、`413` 请求或 CSV 导出结果过大、`429` 限流、`502` provider 失败、`503` 服务未就绪、`500` 内部错误。所有 `/api/*` 错误都返回该 envelope，不返回 Flask 默认 HTML。响应头 `X-Request-ID` 与 envelope 的 `request_id` 用于排障关联；它不代表身份或权限。后端不把 Python traceback、API Key、上游请求头或未经筛选的 provider 响应返回给浏览器。`PROVIDER_RESPONSE_TOO_LARGE` 表示上游 JSON 超过服务端配置的响应上限（默认 2 MB）；`EXPORT_TOO_LARGE` 表示个人或管理员 CSV 超过 100,000 行或 16 MiB 安全边界；`SERVICE_NOT_READY` 表示中心数据库 schema 尚未就绪。

## `GET /api/summary?period=day`

`period` 可选值：`day`、`week`、`month`、`all`。也可以使用 `from=YYYY-MM-DD&to=YYYY-MM-DD` 覆盖周期，结束日期包含当天。

```json
{
  "period": "day",
  "from": "2026-08-10 00:00:00",
  "to": "2026-08-11 00:00:00",
  "totals": {
    "calls": 2,
    "input_tokens": 2000,
    "output_tokens": 470,
    "total_tokens": 2470
  },
  "by_model": [
    {
      "model": "kimi-code",
      "calls": 1,
      "input_tokens": 1200,
      "output_tokens": 350,
      "total_tokens": 1550
    }
  ],
  "trend": [
    { "day": "2026-08-10", "input_tokens": 2000, "output_tokens": 470, "total_tokens": 2470 }
  ],
  "records": []
}
```

## `GET /api/v1/records`

返回当前 bearer 用户的分页记录，不会返回其他用户数据：

```text
GET /api/v1/records?period=month&limit=50&offset=0
```

响应字段为 `period`、`from`、`to`、`records` 和
`pagination(limit, offset, total)`；时间范围继续按本地时间和半开区间处理。

## `POST /api/v1/records`

Android 客户端使用该接口写入手动 token 记录；旧网页会话接口
`POST /api/records` 继续保留兼容。服务端按 bearer 账号归属记录数据，并将
`source` 固定为 `android`，客户端不能伪造归属来源。

Android 必须为一次用户操作生成稳定的 `Idempotency-Key` 请求头；同一用户
重复提交相同 key 会返回第一次写入的记录，并将状态设为 `200`、`replayed: true`。
新的 key 写入返回 `201`、`replayed: false`。服务端不会把该 key 放入记录投影。

请求：

```json
{
  "model": "kimi-code",
  "input_tokens": 1200,
  "output_tokens": 350,
  "timestamp": "2026-08-10T14:30",
  "note": "项目设计讨论"
}
```

请求头示例：`Idempotency-Key: 7f4c1a2e-...`。

返回包含 `record`、服务端生成的 `request_id` 和 `replayed`。输入/输出必须是非负整数，模型不能为空，备注最多 1000 个字符。

## `GET /api/export?period=month`

返回 UTF-8 BOM CSV，字段为：

`id, model, input_tokens, output_tokens, total_tokens, timestamp, note, source`

个人 CSV 同样受最多 100,000 行、16 MiB 的服务端边界保护；超过时不返回部分文件，使用
`413 EXPORT_TOO_LARGE` 错误 envelope。CLI 导出在写入目标文件前完成边界检查。

## `POST /api/provider/models`

用于连接初始化时自动检测模型，不产生 token 记录。

请求：

```json
{
  "provider": "auto",
  "base_url": "https://api.moonshot.cn/v1",
  "api_key": "仅在本次检测内使用"
}
```

成功返回：

```json
{
  "provider": "openai-compatible",
  "models": ["kimi-k2", "kimi-k2-thinking"]
}
```

Key 只用于服务端向上游发送本次 `GET /models`，不写入数据库或响应。模型 ID 来自第三方响应，服务端只接受字符串 ID 并去重排序。

## `POST /api/proxy/chat/completions`

请求由页面生成：

```json
{
  "provider": "auto",
  "base_url": "https://api.moonshot.cn/v1",
  "api_key": "仅在内存中使用",
  "model": "kimi-k3",
  "messages": [{ "role": "user", "content": "你好" }],
  "note": "自动代理调用"
}
```

服务端强制 `stream=false`，只允许命中 `TOKEN_TRACKER_ALLOWED_BASE_URLS` 的 HTTPS Base URL。页面不要求用户手动输入 token 数；上游成功且有 usage 时返回：

provider 请求在边界限制 API Key 4096 字符、Base URL 2048 字符、模型名 200 字符和消息
数组 100 项；总请求体和上游响应仍分别受 256 KB/2 MB 限制。超限返回
`PROVIDER_INPUT_INVALID`，不会回传或记录 API Key。

```json
{
  "recorded": true,
  "usage": { "prompt_tokens": 12, "completion_tokens": 24, "total_tokens": 36 },
  "record": {},
  "response": {}
}
```

上游没有 usage 时返回 `recorded=false` 和 `warning`，不猜测 token 数量。

`provider` 默认为 `auto`，当前解析为 `openai-compatible` adapter；它支持公共的
`GET /models` 与非流式 `POST /chat/completions` 形状。响应会带出实际解析到的
`provider`，方便未来增加专用 Kimi Code、Anthropic 或 Gemini adapter 时保持前后端契约稳定。

上游返回模型优先级：`response.model` → `request.model`。usage 输入别名允许 `prompt_tokens`、`input_tokens`、`promptTokens`；输出别名允许 `completion_tokens`、`output_tokens`、`completionTokens`。只有输入和输出都能通过非负整数校验时才入库。

## Android bearer provider endpoints

Android 使用同一 provider 业务边界，但必须先通过 `/api/v1/auth/login` 获取 bearer access token：

- `POST /api/v1/provider/models`：请求体与浏览器版本相同，返回 `provider` 和 `models`，不写 token 记录。
- `POST /api/v1/proxy/chat/completions`：请求体与浏览器版本相同，必须携带 `Authorization: Bearer <access_token>` 和客户端生成的 `Idempotency-Key`；成功响应允许客户端读取 `response`、`usage`、`recorded`、`replayed`、`warning` 和 `record`。

Android 客户端会在本地把上游 `response` 降维为首个助手文本，把 `usage` 降维为输入/输出/合计摘要；完整 provider 响应不进入 Android 状态、日志或本地持久化。API Key 只存在于一次请求的内存参数中，Windows 服务不把它写入 SQLite、CSV 或审计日志。

## 兼容与演进

- 现有成功字段保持向后兼容；新增字段只能默认可忽略。
- provider 专用逻辑使用 adapter/registry 扩展，不能在 `web.py` 中增加厂商分支。
- 任何删除字段、改变状态码或改变 Key 生命周期的变更必须新增 ADR。
