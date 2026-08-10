# ADR-043：Android bearer 会话同步持久性

## 状态

已接受（2026-08-10）。

## 安全边界

Android 的 access token 和 refresh token 以 Android Keystore 保护的 AES-GCM blob
保存在应用私有容器。威胁不是只读偏好设置，而是登录/刷新/退出之后进程立即终止，
导致内存状态与本地持久化状态不一致：可能丢失新会话，或让退出后的旧 blob 继续存在。

## 决策

- `EncryptedSessionStore` 仍只保存加密 blob，不把 token 拆到普通偏好字段、日志或 Compose 状态。
- `save` 和 `clear` 使用同步 `SharedPreferences.commit()`，并在返回前检查 Boolean 结果；
  失败抛出不含凭据的本地存储异常，不伪装为成功。
- Repository 已保证这些调用发生在单线程 worker 上，因此同步提交不会阻塞 Android UI 线程。
- 清除仍只删除 session key，不影响非秘密的服务地址偏好；服务地址切换、退出和 refresh
  失效路径共享同一清除边界。

## 取舍

- 同步提交比 `apply()` 增加一次本地 I/O 等待，但会把会话生命周期的成功语义变成可验证的
  durable boundary；登录和刷新本就发生在网络 worker 上，成本可接受。
- 该措施不能防御已 root 设备、内核级窃取或应用进程内存读取；它只收紧应用级崩溃/终止窗口。

## 验证

- 静态确认 Android 端仅 `EncryptedSessionStore` 触碰 session blob，未新增明文 token 字段。
- Kotlin 文件行数、源文件头/KDoc、Android Release HTTPS 配置和 no-download 构建门禁继续通过。
- 当前机器无 JDK/Gradle/SDK，APK 编译、安装和设备级 Keystore 演练仍需用户批准工具链后执行。
