# ADR-098：用户授权的 Android SDK 一键准备入口

**作者：** AI Token Tracker Engineering Team  
**维护者：** DEV-4 / ARCH-2  
**状态：** Accepted；SDK license/APK 仍 pending  
**日期：** 2026-08-10  
**关联：** ADR-081、ADR-085

## 背景

Android 工具链已经固定在项目目录，但 API 37 和 Build Tools 需要用户在 Google
SDK 交互提示中确认 license。直接把长 PowerShell 命令复制给使用者容易造成路径错误，
也容易让人误以为项目可以代替用户接受第三方协议。

## 决策

新增 `android/accept-sdk-license.bat` 作为 Windows 用户入口。它只负责编排既有边界：

1. 检查项目内 `sdkmanager.bat`，不回退到全局 SDK。
2. 要求用户明确输入 `ACCEPT`，随后进入官方 `sdkmanager --licenses` 交互提示。
3. 复用 `provision-toolchain.ps1 -InstallSdkPackages` 安装固定的 Platform Tools、
   API 37 和 Build Tools 37.0.0。
4. 复用 `toolchain-doctor.ps1`，通过后输出 `build-local.bat assembleDebug`。
5. 安装前要求项目内 `licenses/android-sdk-license` 存在且包含非空许可内容；仅有
   `licenses` 目录但没有实际授权文件时必须 fail-closed。

脚本不自动输入 `y`、不改全局 PATH、不写入许可证内容、不下载未锁定版本，也不生成
APK 假证据。用户取消或拒绝 license 时返回明确的非零退出码。

## 验证与边界

脚本头部、行数、PowerShell/批处理调用和 Android 工具链契约纳入 `token_tracker audit`
与 `ci/quality-gate.ps1`。当前机器仍因 license 未确认而保持 Android toolchain pending；
本 ADR 不表示 APK 已编译、安装或完成设备联调。

## 回滚

删除 `android/accept-sdk-license.bat`、本 ADR 及其引用即可回退；不删除项目内 JDK、
Gradle、SDK 或用户已明确安装的合法工具链。
