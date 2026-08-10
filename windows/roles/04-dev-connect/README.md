# DEV-3 Connect / Frontend State

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**职责：** 模型自动检测、代理调用、连接状态、Key 生命周期和事件上报。

## 安全边界

- Key 只存在当前调用链的内存中，调用结束或页面刷新即清除。
- 不在客户端持久化 Key；服务端不把 Key 写入 SQLite、审计事件、CSV 或错误响应。
- provider 只能命中服务端白名单，所有 usage 以 provider 返回值为准。
- 自动采集失败必须保留可读错误码；禁止用估算值伪造 token。

