# 企业级工程与注释规范

**作者（Author）：** AI Token Tracker Engineering Team  
**维护者（Maintainer）：** Project Owner  
**适用范围：** 当前 checkout 的全部源代码、模板、静态资源、文档和角色交付包  
**状态：** Accepted

## 1. 作者与文件头

项目未提供个人作者姓名时，统一使用 `AI Token Tracker Engineering Team`，避免不同代理虚构个人署名。后续项目所有者可以在一次独立变更中替换为真实姓名，不改变代码行为。

生产代码文件必须在顶部保留简短元数据：

```text
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: <one-line responsibility>
```

Python 将元数据放入模块 docstring；JavaScript/CSS/HTML 使用文件头注释；Markdown 使用文档元信息。不要在每个函数重复作者名。

## 2. 代码边界

```text
templates/static → JSON API → web.py → services.py → db.py → SQLite
                                ↓
                         providers.py → allowlisted upstream
```

- `web.py` 只编排 HTTP、会话和响应，不拼接 SQL，不承担 provider 解析细节。
- `providers.py` 只负责外部 provider 的 URL、请求、响应解析和 adapter 边界，不写用户数据库。
- `services.py` 负责用例校验和领域动作，不渲染 HTML。
- `db.py` 负责参数化 SQL、聚合和持久化，不读取 HTTP request。
- 前端只通过 API client 访问后端；模板不直接依赖数据库结构。
- 新增跨层功能先写 API/类型/状态契约，再实现后端和前端。

## 3. 注释与 docstring

注释解释“为什么”、安全边界、失败语义和不可见的业务约束，不翻译下一行代码。

```python
# Good: the provider response is authoritative because aliases may route a
# requested model to a family-specific deployment; guessing here corrupts data.
model = response_json.get("model") or requested_model
```

禁止：

- `# increment counter` 这类重复代码含义的注释；
- 注释掉的旧代码；
- 没有对应任务或 ADR 的长期 TODO；
- 用注释掩盖不清晰的类型边界。

复杂逻辑优先拆成命名函数，注释说明该函数的稳定不变量。

## 4. API 与错误

公开 JSON 接口使用稳定字段名、显式状态码和统一错误 envelope。新增字段只能优先采用可选字段；删除或改变字段必须新增 ADR，并提供迁移说明。第三方响应先校验形状，再进入业务逻辑或页面。

## 5. 安全

- API Key 不进入 SQLite、CSV、日志、异常文本、localStorage 或响应头回传。
- 所有用户输入在 HTTP 边界校验；SQL 一律参数化；页面输出使用 text/模板转义。
- 外部 URL 必须 HTTPS、白名单、超时、禁止自动重定向并限流；禁止把用户输入变成开放代理。
- 外部响应也必须有可配置的大小上限；解析 JSON 前按 chunk 读取，不能只依赖浏览器请求大小限制。
- provider 原始响应必须经过 `provider_projection.py` 才能进入 Web/Android 公共响应；不得把
  隐藏推理、工具参数、供应商 metadata 或请求回显直接透传给客户端。
- 所有写接口都需要会话授权和 CSRF；错误响应不得暴露 traceback。
- 不提交 `.env`、SQLite、密码、真实 Key 或个人日志。

## 6. 前端和动效

- 设计 token 使用 `surface/on-surface/on-surface-variant/outline/primary` 语义，而不是在组件中散落任意 hex。
- 正文和表单普通文本目标对比度至少 4.5:1；大标题至少 3:1；状态同时提供文字，不只依赖颜色。
- 动效必须有目的、可取消、可降级；`prefers-reduced-motion` 下信息和交互完整保留。
- 交互组件默认使用语义 HTML；键盘 focus-visible 明确可见；加载、错误、空状态不能是空白页面。

## 7. 交付检查

每个角色交付包都必须提供：变更目的、触及文件、契约影响、风险、验证证据和未完成项。架构师合并前执行五轴检查：正确性、可读性、架构、安全、性能。

## 8. 文件规模门禁

任何文件最多 1000 行。接近 800 行时必须开始拆分，超过 1000 行不得合并。拆分必须围绕单一职责，并用显式导出、类型契约或 HTTP API 暴露边界；不允许复制代码、循环依赖或依赖隐式全局状态来“拆分”。交付记录必须附上变更文件的行数扫描结果。

## 9. 跨端与企业级注释

- 每个源文件头必须包含 `Author`、`Maintainer`、`Purpose` 和模块归属；Kotlin/Java 的公开类与方法使用 KDoc，Python 的公开接口使用 docstring，前端模块使用 JSDoc。
- 接口文档必须写明输入、输出、权限、错误码、数据敏感级别和兼容策略；网页与 Android 不得各自解释同一个字段。
- 注释只记录设计原因、边界、不变量和风险，不重复代码，不留下失效实现和无期限 TODO。
- UI、HTTP、业务、数据、provider、Android data/domain/ui 层按单向依赖连接；当一个文件需要知道两个不相邻层的实现细节时，优先新增接口或服务边界。
