# ADR-006：Android 使用 Kotlin + Jetpack Compose，并复用 `/api/v1`

## Status

Accepted for the first mobile client.

## Context

产品需要 Android APK 与网页共享账号、查看用量和工作事件，同时保留高质量的动态视觉表现。

## Decision

Android 采用 Kotlin + Jetpack Compose，使用 UI/data 分层和 ViewModel/Repository 边界；认证和数据通过 Windows 服务的 `/api/v1` 完成。客户端只保存最小的会话材料，并使用 Android 平台安全存储，不保存 provider API Key。

## Alternatives

- WebView：首版快，但无法提供稳定的原生动效、离线缓存和系统体验。
- Flutter/React Native：跨端复用更强，但当前只要求 Android，增加另一套运行时和视觉调优边界。
- 直接把 SQLite 同步到手机：会破坏中心化事实源和管理员审计。

## Consequences

- 需要 Android Studio、JDK 17、Android SDK、Gradle 和模拟器/实体设备。
- API 契约必须独立于 Flask 模板，以支持网页和 Android 并行演进。
- APK 的服务器地址必须可配置，开发模拟器、局域网设备和生产 HTTPS 域名不能混为一谈。

