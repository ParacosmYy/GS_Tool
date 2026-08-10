# Android 客户端开发环境

**作者：** AI Token Tracker Engineering Team  
**状态：** Android 第一条联动竖切片已完成；项目内 v11 场景已接入，工具链构建门禁单独跟踪

## 选型

- Kotlin + Jetpack Compose：适合动态仪表盘、可组合组件和高质量动效。
- 单 Activity + Compose 屏幕；当前首切片使用明确的 auth/dashboard feature 边界。
- UI / data 分层，ViewModel 作为状态持有者，Repository 作为 API 数据边界。
- Android 与网页只共享 `/api/v1` 契约，不共享 SQLite 文件。
- 第一阶段只使用平台 `HttpURLConnection` 与 Android Keystore，避免在未获用户批准前下载特殊依赖。

这是 Android 官方推荐的方向：Compose 项目使用 Kotlin，建议 API 21 或更高；官方架构指南强调 UI/data 分层、单一事实源和单向数据流。

## 当前机器检查结果

检查日期：2026-08-10（Asia/Shanghai）

- `adb`：已安装，但当前不属于项目内 Android SDK；只能用于只读设备检查，不能替代构建 SDK。
- JDK 17：未发现。
- Gradle：未发现。
- Android Studio：未发现。
- Android SDK 环境变量：未发现；项目预留路径为 `android/.toolchain/android-sdk/`。

因此当前可以编辑工程和 API 契约，但不能在这台机器上声称 APK 已成功编译。安装 Android Studio/JDK/Gradle/SDK 属于用户批准的环境变更；在批准前不下载或自动配置这些工具。安装 Android Studio 后，把 SDK 包安装到 `android/.toolchain/android-sdk/`；AGP 9.3.0 的官方兼容要求是 Gradle 9.5.0、JDK 17，最大支持 API 37。Wrapper 配置已经把发行包路径放到 `android/.gradle/`，官方 Wrapper 文件尚未生成。

## 建议安装顺序

1. 安装最新版 Android Studio。
2. 在 Setup Wizard 中安装 Android SDK、Platform Tools、Build Tools 和一个 API 级别 36/37 的模拟器镜像。
3. 配置 `JAVA_HOME` 指向 Android Studio 自带的 JDK 17，或独立 JDK 17。
4. 在 Android Studio 打开本目录，执行 Gradle Sync。
5. 在 `android/local.properties` 中由 IDE 写入本机 SDK 路径；此文件不提交。
6. 连接实体设备或启动模拟器，先验证登录和只读仪表盘，再接入写入/同步能力。

命令行构建前先执行项目内只读诊断：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\android\toolchain-doctor.ps1
```

诊断不会下载、安装、修改 PATH 或生成 wrapper；只有 JDK、官方 Wrapper、项目内 SDK API 37/build-tools 完整后才允许进入 `android/build-local.bat assembleDebug`。该入口把 `GRADLE_USER_HOME`、`ANDROID_USER_HOME` 和 `ANDROID_SDK_ROOT` 指向 Android 项目目录，因此 Gradle 发行包、Maven 缓存、Android 元数据和 SDK 都能留在项目边界；Android Studio 的 Sync 仍需在工具链批准后单独核对其缓存策略。Release 必须显式传入 HTTPS 地址：

```powershell
android\build-local.bat assembleRelease -PtrackerApiBaseUrl="https://your-host.example/api/v1"
```

Gradle 配置阶段会拒绝 `http://` 或默认模拟器地址，Manifest 的 `usesCleartextTraffic=false`
继续作为运行时第二道门禁；Debug 才允许本机模拟器 HTTP。

## 当前已完成的 Android 联动

- `TrackerRemoteDataSource`：稳定的远程接口，覆盖登录、刷新、退出、汇总、事件、日志和管理员只读边界。
- `JsonHttpDataSource`：解析 `/api/v1` 结构化响应，限制响应大小、超时和重定向。
- `EncryptedSessionStore`：用 Android Keystore + AES/GCM 保存加密会话 blob。
- `TrackerRepository`：统一 access token 重试、refresh token 轮换和本地退出清理。
- `TrackerViewModel`：恢复会话并驱动登录、刷新、错误和退出状态。
- Compose UI：登录、普通成员 token 记录写入、今日汇总、管理员团队总览、趋势、模型占比、成员按需明细、空态与错误态。
- 视觉资源：Windows 与 Android 共用的 `embedded-rust-engineer-bg-v11`，右侧为成年御姐气质、可爱但非性化的嵌入式 Rust/RL 工程师，手持银色专业笔记本，工作台放置独立紧凑银色桌面工作站、示波器与实验板，左侧保留仪表盘留白；两端都使用渐变遮罩保证文字可读性，Android 背景增加低幅度漂移动画，v10/v9/v8 及之前版本仍保留回滚，资产决策见 ADR-080。

## 项目内技能资料

Android 官方技能资料安装在 `android/skills/`，只服务本项目，不写入全局 Codex 配置。当前保留 adaptive、styles、edge-to-edge 三组资料；在没有完成 Gradle 验证前，不引入需要额外依赖的实验 API。

## 联动配置

开发环境的服务地址不能写死为 `127.0.0.1`。Android 登录页允许用户输入并记住非敏感的 `/api/v1` 地址：

- Android 模拟器访问 Windows 主机通常使用 `10.0.2.2` 映射宿主机。
- 实体手机需要访问 Windows 主机的局域网 IP，并在防火墙放行 HTTPS 端口。
- 生产环境必须使用 HTTPS 域名；禁止把 API Key、密码或 refresh token 写入 APK 源码。
- 更换服务地址时客户端清理旧会话，避免跨中心服务复用 bearer token。
- 端点校验拒绝 userinfo、query 和 fragment；API Key 只在用户触发上游调用的内存生命周期内存在。
- Android 手动 token 记录通过 `/api/v1/records` 写入，并可通过同一路径分页读取；服务端固定 `source=android`，成功后重新读取 `/me/summary`，不在客户端自行计算汇总。

端点输入与会话切换的取舍记录在 [`ADR-010`](decisions/ADR-010-android-endpoint-configuration.md)。

Release HTTPS 构建门禁记录在 [`ADR-041`](decisions/ADR-041-android-release-https-gate.md)。

管理员只读边界、字段投影和按需加载策略记录在 [`ADR-011`](decisions/ADR-011-android-admin-readonly.md)。

版本化 token 写入边界记录在 [`ADR-012`](decisions/ADR-012-versioned-usage-write.md)。

## 相关官方依据

- Android Studio 安装与系统要求：<https://developer.android.com/studio/install>
- Jetpack Compose 快速开始：<https://developer.android.com/develop/ui/compose/setup>
- Android 应用架构：<https://developer.android.com/topic/architecture>
- Compose Compiler Gradle Plugin：<https://developer.android.com/develop/ui/compose/setup-compose-dependencies-and-compiler>
- Android Gradle Plugin 版本与兼容性：<https://developer.android.com/build/releases/about-agp>
- AGP 9.3.0 发布说明与兼容矩阵：<https://developer.android.com/build/releases/agp-9-3-0-release-notes>
- Compose BOM 版本管理：<https://developer.android.com/develop/ui/compose/bom>
- Android 17 SDK / API 37 配置：<https://developer.android.com/about/versions/17/setup-sdk>
