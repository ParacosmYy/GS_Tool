# ADR-015：Android 复用 bearer provider 边界自动采集

**状态：** Accepted  
**日期：** 2026-08-10  
**范围：** Windows `/api/v1`、Android `data/remote`、普通成员仪表盘

## 背景

Android 已经可以使用同账号读取汇总和写入手动记录，但如果 provider 调用仍停留在 Windows 网页端，移动端无法完成“检测模型 → 发起调用 → 真实 usage 自动记账”的联动闭环。

## 决策

1. 新增 bearer 版本的 `/api/v1/provider/models` 与 `/api/v1/proxy/chat/completions`，复用 Windows 的 provider adapter、白名单、超时、响应大小和限流边界。
2. Android 通过 `TrackerRemoteDataSource` 与 `TrackerRepository` 调用接口；Compose 只接收模型列表和安全结果投影，不接触 SQLite 或 HTTP 细节。
3. Android API Key 只存在当前 Compose 表单的普通内存状态和一次请求调用栈中，不使用 `rememberSaveable`，不进入 ViewModel、日志、记录备注或保存状态。
4. 自动调用使用客户端生成的 `Idempotency-Key`；服务端按用户和幂等键保证网络重试不会重复写入 usage。
5. provider 原始响应只在请求链路中被消费；Android 仅展示首个助手文本、usage 摘要和服务端记录投影。

## 取舍与后续

- 当前只支持 OpenAI-compatible 的非流式接口；没有 usage 时明确提示，不猜测 token 数。
- 真正的 Kimi Code、Anthropic 或 Gemini 专用格式继续通过 `ProviderAdapter` 扩展，不在 Android UI 中堆厂商分支。
- APK 构建和真机联调仍受项目内 JDK/Gradle/SDK 尚未获批的环境门禁约束。
