# ADR-041：Android Release 构建强制 HTTPS 服务地址

## 状态

已接受（2026-08-10）。

## 背景

Android Debug 需要 `http://10.0.2.2:5000/api/v1` 访问本机模拟器宿主服务，Release 则必须
通过中心 HTTPS 网站与 Windows 服务联动。仅在 Manifest 关闭明文流量不能阻止开发者把
模拟器地址或 HTTP 配置写入 APK 的 BuildConfig，错误配置会到运行时才暴露。

## 决策

1. `android/app/build.gradle.kts` 保留 Debug 默认地址，但 Release 配置阶段要求
   `trackerApiBaseUrl` 以 `https://` 开头；未显式传入安全地址时直接失败。
2. Release Manifest 继续使用 `usesCleartextTraffic=false`，作为构建期门禁之外的运行时防线。
3. Android 文档统一使用 `android/build-local.bat assembleRelease -PtrackerApiBaseUrl=...`，
   不把真实域名、密码或 API Key 写入仓库。

## 取舍

- Debug 的零配置模拟器体验保持不变；实体设备和分享部署仍需用户输入 Windows HTTPS 地址。
- 该门禁只验证传输协议前缀，不替代真实证书、域名、Caddy、ACL 和设备联调验证。

## 验证

- 源码检查确认 Release 配置包含 HTTPS fail-fast，Debug 默认仍为 `10.0.2.2`。
- `android/build-local.bat assembleDebug` 在缺 Wrapper 时按预期 fail-closed；当前机器未获
  JDK/Gradle/SDK 批准，因此未声称 APK 构建成功。
