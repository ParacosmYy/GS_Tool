# ADR-081：Android 项目内可复现工具链与 AGP 9 迁移

**作者：** AI Token Tracker Engineering Team
**维护者：** ARCH-2 / DEV-4
**状态：** Accepted
**日期：** 2026-08-10
**作用：** 固化 Android 工具链的项目边界、下载完整性、license 边界和 AGP 9 构建兼容规则。

## 背景

Android 工程需要一个同学可以按文档复现的 Debug APK 入口，但开发机没有 Android Studio、JDK、Gradle
发行包或 SDK。工具不能静默污染全局 PATH，也不能把 Google SDK license 的法律确认替用户完成。首次
生成官方 Wrapper 时还暴露了两处 AGP 9 迁移问题：过期的 BuildConfig 全局开关，以及不再需要的
`org.jetbrains.kotlin.android` 插件。

## 决策

1. 将 JDK 17、Gradle 9.5.0、Android command-line tools 和下载归档固定在 `android/.toolchain/`；这是
   project-local toolchain，不依赖全局 PATH。
   Gradle/Maven/Android 用户缓存固定在 `android/.gradle/`。这些目录不进入 Git。
2. `android/provision-toolchain.ps1` 只下载固定 URL 的归档并校验 SHA-256；它不会修改全局 PATH，
   不会上传归档，也不会自动向 `sdkmanager --licenses` 输送同意内容。
3. 只读 `toolchain-doctor.ps1` 优先检查项目内 JDK，再检查 `JAVA_HOME`/PATH；官方生成的
   `gradlew.bat`、`gradlew` 和 `gradle-wrapper.jar` 纳入 Git，保证入口本身可复核。
4. `platforms;android-37` 与 `build-tools;37.0.0` 只有在用户已交互接受 SDK license 后，才能通过
   `provision-toolchain.ps1 -InstallSdkPackages` 安装；未接受时必须保持 pending。
5. AGP 9.3 使用内置 Kotlin：移除重复的 `org.jetbrains.kotlin.android`，保留 Compose 编译器插件；
   移除已被 AGP 9 删除的 `android.defaults.buildfeatures.buildconfig`，由 app 的
   `buildFeatures.buildConfig = true` 负责生成字段。Release HTTPS 校验只在明确的 Release 任务上
   触发，避免误伤 Wrapper/Debug 配置阶段。

## 固定来源与完整性

| 组件 | 来源 | SHA-256 | 项目路径 |
| --- | --- | --- | --- |
| Temurin JDK 17.0.20+8 | `https://github.com/adoptium/temurin17-binaries/releases/download/jdk-17.0.20%2B8/OpenJDK17U-jdk_x64_windows_hotspot_17.0.20_8.zip` | `418497be5cf585bdd2203d6486a565d66d3f5e992d5630d45104cb873fab8122` | `.toolchain/jdk-17/` |
| Gradle 9.5.0 | `https://services.gradle.org/distributions/gradle-9.5.0-bin.zip` | `553c78f50dafcd54d65b9a444649057857469edf836431389695608536d6b746` | `.toolchain/gradle-9.5.0/` |
| Android command-line tools | `https://dl.google.com/android/repository/commandlinetools-win-15859902_latest.zip` | `90ae805d20434428bffcb699c290860f19bb5f66a67e6b330067e3de801fb04a` | `.toolchain/android-sdk/cmdline-tools/latest/` |

## 官方依据

- [AGP 9.3.0 兼容性与 API 37](https://developer.android.com/build/releases/agp-9-3-0-release-notes)
- [Android 命令行工具下载](https://developer.android.com/studio)
- [Gradle Wrapper 官方文档](https://docs.gradle.org/9.5.0/userguide/gradle_wrapper.html)
- [Temurin 17 发布页](https://adoptium.net/temurin/releases/?version=17)

## 验证与边界

- 已验证 JDK `17.0.20`、项目 Gradle `9.5.0`、command-line tools `22.0` 和官方 Wrapper 生成成功。
- 当前 doctor 结果为 JDK、Wrapper、Gradle contract、ADB 通过；API 37/Build Tools 仍因 license 尚未
  交互确认而 pending，APK 尚未宣称构建成功。
- 真实设备安装、同账号联调、Release APK 和 Android Studio 视觉验收仍属于 R-08/Android 门禁。

## 回滚

删除项目内被忽略的 `.toolchain/` 与 `.gradle/` 可回收本机缓存；恢复 `gradle.properties`、插件声明和
Release 任务条件即可回到迁移前源码。不要删除已提交的 Wrapper 文件，除非同步撤销整个工具链决策。
