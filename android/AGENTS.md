# Android 客户端约束

本目录是独立 Android 产品。根目录 `AGENTS.md` 的安全、文件规模和角色约束同样适用。

- 使用 Kotlin + Jetpack Compose；UI、状态、数据访问分层，不把网络请求写进 Activity。
- 每个文件最多 1000 行；接近上限按 feature、data、ui、theme 等高内聚职责拆分，并通过显式接口连接。
- Android 只通过 Windows 服务的 `/api/v1` 访问数据，不直接读取 SQLite。
- 密码、API Key、refresh token 不写入源码、日志或普通 SharedPreferences；会话材料使用 Android 安全存储方案。
- `http://10.0.2.2` 只允许开发模拟器调试；发布构建必须使用 HTTPS。
- 当前先交付可导入工程和视觉壳，网络认证、管理员权限和日志同步必须等 API 契约冻结后实现。

## 企业级注释要求

- Kotlin 源文件顶部必须说明 `Author`、`Maintainer`、`Purpose` 和所属层；公开的 Activity、Composable、ViewModel、Repository、DTO 和 API client 必须有 KDoc。
- KDoc 说明状态来源、生命周期、输入输出、线程/协程约束、错误处理和安全边界；不要把代码逐行翻译成注释。
- 每个 feature 只能通过 data/domain/ui 的显式接口连接；Composable 不直接创建 HTTP 客户端，Activity 不持有跨重建的业务状态。
- 任何跨端字段、错误码和事件必须引用 `windows/docs/api-contract.md` 或对应 ADR；发现契约缺口先补文档再写实现。
