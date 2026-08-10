# ADR-009：Android 工具链门禁与首条联动竖切片

- **状态：** Accepted
- **日期：** 2026-08-10
- **作者：** AI Token Tracker Engineering Team
- **范围：** `android/` 客户端与 `windows/token_tracker/api_v1.py`

## 背景

项目需要让 Windows 中心服务、网页和 Android 客户端使用同一个账号。当前开发机只有 `adb`，没有 JDK、Gradle、Android Studio 或 Android SDK 管理工具。用户要求所有环境依赖留在项目内，并明确不允许在未批准时下载特殊依赖。

## 决策

1. 先冻结并实现 `/api/v1` 的认证、个人汇总、事件、日志和分页契约；Android 只依赖该契约，不直接读取 Windows SQLite。
2. Android 首条竖切片使用 Kotlin + Jetpack Compose、平台 `HttpURLConnection`、Android Keystore AES/GCM、ViewModel 和 Repository，不在当前阶段引入新的网络库或实验性导航依赖。
3. 所有 Android 工具链安装、SDK 下载和 APK 构建作为显式环境门禁：在用户批准前只做源码、契约、静态检查和现有 Windows 运行证据，不声称 APK 已构建。Gradle Wrapper 的发行包与缓存固定在项目内 `android/.gradle/`；在缺少官方生成的 wrapper JAR/脚本时不手工伪造。
4. 后续允许替换 HTTP 实现时，只替换 `TrackerRemoteDataSource` 的适配器；认证、分页、幂等、错误信封和 UI 状态不跨层泄漏。
5. 项目级 Android 技能资料保存在 `android/skills/`，不修改全局 Codex skill 配置；实验性 API 只有在工具链验证后才可进入生产代码。
6. Android 服务地址由登录页配置，端点生命周期与会话切换规则单独记录在 ADR-010；默认地址仍由 `BuildConfig.API_BASE_URL` 提供。

## 结果

- 已完成 Android 登录、加密会话恢复、今日汇总、趋势、模型占比、最近记录、刷新、退出和结构化工作信号写入的接口与 UI 边界。
- Windows `/api/v1/events/work` 与 `/api/v1/logs` 支持 `limit`、`offset` 和 `pagination`，并保持旧列表字段兼容。
- 未完成 APK 构建与设备联调；其前置条件是用户批准并安装 JDK 17、Gradle/Gradle Wrapper、Android SDK/Build Tools 和可用模拟器或设备。

## 安全与运维约束

- 开发模拟器可使用 `http://10.0.2.2:5000/api/v1`；实体设备和发布版本必须使用可验证的 HTTPS 地址。
- 不把 provider API key、密码、access token 或 refresh token 写入源码、日志、APK 资源或普通明文偏好设置。
- 真实公网分享前必须补齐 HTTPS、备份、限流、审计、隐私告知和恢复演练；本 ADR 不代表生产部署已完成。
