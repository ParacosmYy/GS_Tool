# AI Token Tracker Android

Android 端与 `windows/` 中心服务使用同一个账号和 `/api/v1` 契约。当前提交已经完成登录、加密会话恢复、汇总、趋势、活动历史、个人 token 记录读写、provider 模型检测、provider 调用自动记账、刷新、退出和结构化工作信号写入；管理员账号还可以进入团队只读观测台。

## 打开项目

用最新版 Android Studio 打开本目录，等待 Gradle Sync。工程基线使用：

- Android Gradle Plugin 9.3.0
- Gradle 9.5.0
- JDK 17
- Kotlin 2.3.21 / Compose Compiler Gradle Plugin
- Compose BOM 2026.06.00
- compileSdk / targetSdk 37，minSdk 23

这些版本以 2026-08-10 核对的 Android/Gradle 官方文档为依据；如果 Android Studio 提示稳定版升级，优先使用 IDE 的升级助手并同步修改版本说明。Wrapper 的发行包与缓存路径已指向项目内 `android/.gradle/`；Android SDK 位于 `android/.toolchain/android-sdk/`。本机已按 ADR-081 配置项目内 JDK 17、Gradle 9.5.0 和 command-line tools，并已生成官方 Wrapper；SDK API 37/Build Tools 仍需用户交互确认 license 后安装，APK 尚未宣称构建完成。

官方版本依据：

- [AGP 9.3.0 兼容性](https://developer.android.com/build/releases/agp-9-3-0-release-notes)：Gradle 9.5.0、JDK 17、API 37。
- [Compose BOM 2026.06.00](https://developer.android.com/develop/ui/compose/bom)：Compose 依赖使用 BOM 统一版本。
- [Android 17 SDK 配置](https://developer.android.com/about/versions/17/setup-sdk)：`compileSdk`/`targetSdk` 37 与 Build Tools 37 安装路径。

## 本机环境

当前机器检查到 `adb`；项目内 JDK、Gradle、command-line tools 和官方 `gradlew.bat`/`gradlew`/`gradle-wrapper.jar` 已准备完成。Android Studio 仍未安装，但命令行构建不依赖 IDE；SDK API 37/Build Tools 尚未安装，因为 SDK license 必须由用户交互接受。安装顺序、哈希、用户批准门槛和官方链接见 [`windows/docs/android-development.md`](../windows/docs/android-development.md) 与 ADR-081。

工具链入口完成后，先运行只读诊断：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\toolchain-doctor.ps1
```

如果需要在新 checkout 复现项目内工具链，可运行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\provision-toolchain.ps1 -GenerateGradleWrapper
```

首次安装 SDK 前，必须由用户在交互终端确认 license；脚本不会自动代签：

```powershell
$sdk = (Resolve-Path .\.toolchain\android-sdk).Path
$manager = Join-Path $sdk 'cmdline-tools\latest\bin\sdkmanager.bat'
& $manager --sdk_root=$sdk --licenses
powershell -NoProfile -ExecutionPolicy Bypass -File .\provision-toolchain.ps1 -InstallSdkPackages
```

为了减少复制路径，可以直接运行项目内入口；它仍要求你先输入 `ACCEPT`，再进入官方
license 交互提示，不会静默同意第三方协议：

```powershell
.\accept-sdk-license.bat
```

doctor 全部通过后再使用 `android/build-local.bat assembleDebug`。该入口会把 Gradle 用户目录、Android 用户元数据、发行包和依赖缓存指向 `android/.gradle/`，并拒绝使用未配置的外部 Android SDK。

## 开发 API 地址

登录页会要求填写 Windows 服务地址；首次启动默认使用：

```text
http://10.0.2.2:5000/api/v1
```

实体手机不能使用 `10.0.2.2`，需要在登录页替换成 Windows 主机的局域网 HTTPS 地址。地址会保存在本机普通偏好设置中，密码、access token 和 refresh token 不会与它混存；更换服务地址会主动清除旧会话，避免把旧服务的令牌发到新服务。

Debug 构建时可以设置初始默认地址；Release 构建会在 Gradle 配置阶段拒绝 HTTP 地址：

```powershell
android\build-local.bat assembleDebug -PtrackerApiBaseUrl="http://10.0.2.2:5000/api/v1"
android\build-local.bat assembleRelease -PtrackerApiBaseUrl="https://your-host.example/api/v1"
```

Release 不允许使用默认的 `10.0.2.2` 或任意 `http://` 地址；这条门禁与 Manifest
的 `usesCleartextTraffic=false` 双重生效，避免把开发配置误打进可分享 APK。

开发 HTTP 仅为本机模拟器临时调试，发布版本必须 HTTPS；不要把真实 API Key 写进 `gradle.properties` 或 APK。

服务地址只允许 `http`/`https`、主机名和路径，不允许用户名、密码、query 或 fragment。Debug
模拟器可以使用 `http://10.0.2.2`；Release 会在写入 endpoint 或发起请求前拒绝任何 HTTP。
实体设备与公网分享必须使用 HTTPS，并在 Windows 防火墙、反向代理和证书层完成最小暴露。

## 目录

```text
android/
  app/src/main/java/com/aitokentracker/
    MainActivity.kt          # Android 入口
    data/                     # Repository、HTTP DTO、加密会话存储
    feature/auth/             # 登录状态机与登录界面
    feature/dashboard/       # 普通成员观测台
    feature/admin/           # 管理员团队只读观测台
    ui/                       # Compose 壳和动效原语
    MotionPreferences.kt      # Android 系统减少动画策略边界
    ui/theme/                 # 颜色、字体、Material 主题
  app/src/main/res/drawable-nodpi/embedded_rust_engineer_bg_v14.png
                            # 嵌入式 Rust/RL 工程师与 Pro 工作站品牌背景（v14）
  .toolchain/jdk-17/       # 项目内 JDK 17（忽略，不提交）
  .toolchain/gradle-9.5.0/ # 项目内 Gradle 发行包（忽略，不提交）
  .toolchain/android-sdk/  # command-line tools/SDK（忽略，不提交）
  provision-toolchain.ps1  # 固定 URL/SHA-256 的工具链入口
  .toolchain/android-sdk/    # 获批后安装 Android SDK 的项目内位置
  skills/                   # 项目内 Android Compose 技能资料
  app/build.gradle.kts
  settings.gradle.kts
  build.gradle.kts
```

## 当前联动边界

- 模拟器默认请求 `http://10.0.2.2:5000/api/v1`；实体手机必须替换为 Windows 主机局域网 HTTPS 地址。
- Android 端暂时使用平台 `HttpURLConnection`，避免在未获批准前下载网络依赖；Repository 已预留未来替换为 OkHttp/Ktor 的接口边界。
- access/refresh token 只以 Android Keystore 加密 blob 形式落入本地容器；登录轮换和退出清除在
  worker 线程同步提交后才返回，页面、日志和普通明文偏好设置不接触 token。
- 服务端明确拒绝 refresh token 时清除本地加密会话；网络超时不会清除会话，便于恢复后重试。
- 写入事件使用服务端的 `protocol_version`、`command`、`request_id` 和 `Idempotency-Key` 约束；token 记录也会为一次操作生成稳定 `Idempotency-Key`，避免 bearer 刷新或网络重试重复计数。
- 仪表盘的“记录 token 用量”只允许模型、输入/输出 token 和备注，服务端按 `/api/v1/records` 执行统一校验并固定来源；成功写入后客户端自动刷新汇总。客户端同步时还会通过同一路径的 `GET` 分页读取最近个人记录，服务端仍负责范围和用户隔离。
- 仪表盘的“记录工作信号”只允许方向、结果、效率评分和备注，服务端会继续执行白名单与脱敏校验；成功写入后客户端自动刷新汇总。
- 仪表盘会按分页契约读取最近工作事件和诊断日志，当前移动端只展示最近 20 条，避免无界拉取。
- 仪表盘“自动采集”调用 `/api/v1/provider/models` 与 `/api/v1/proxy/chat/completions`；API Key 只由当前表单以内存参数传入，Android 仅解析助手文本、usage 摘要和记录投影，不保存完整 provider 响应。
- 管理员端只读取 `/admin/overview`、`/admin/users` 和选中成员的 `/admin/users/<id>/records`；成员明细按需加载，服务端继续执行 RBAC、字段脱敏和审计记录。
- Android 不保存或展示密码哈希、访问令牌、refresh token、上游 API Key、原始 prompt 或完整 provider 响应；管理员 CSV 导出仍保留在 Windows 网页端。
- 品牌背景和 `SignalOrbit` 遵守 Android 系统动画缩放设置；关闭系统动画时只渲染静态装饰，不改变业务内容或网络状态，详见 ADR-079。
