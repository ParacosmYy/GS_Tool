# ADR-079：Android 装饰动效遵守系统减少动画设置

**作者：** AI Token Tracker Engineering Team  
**维护者：** UI-2 / UI-3 / Android 工程师  
**状态：** Accepted  
**日期：** 2026-08-10  
**范围：** Android Compose 背景场景、SignalOrbit、系统可访问性边界

## 背景

Web 端已经通过 `prefers-reduced-motion` 关闭指针跟随、背景扫描和不必要的持续动画，但 Android
端的品牌背景漂移与信号轨道此前会在所有设备上启动无限过渡。这样会增加低性能设备的绘制成本，
也不尊重用户在系统层关闭动画的意图。

## 决策

1. 新增 `ui/MotionPreferences.kt` 作为唯一 Android 系统动效偏好读取边界。
2. 读取 Android `ANIMATOR_DURATION_SCALE` 与 `TRANSITION_ANIMATION_SCALE`；任一值为 `0` 时，
   只允许静态装饰层渲染。
3. `TokenTrackerApp` 的品牌场景和 `SignalOrbit` 分别选择静态/动态 Composable；静态分支不得创建
   `rememberInfiniteTransition`，不改变业务状态、内容顺序、网络请求或焦点行为。
4. 偏好只在 Compose 组合建立时采样，不启动 observer 或轮询循环；用户从系统设置返回并重新创建页面
   后重新采样。该策略把平台设置耦合限制在 UI 层，避免污染 ViewModel/Repository。

## 后果

普通模式仍保留低频漂移和 18 秒轨道，品牌主题不变；减少动画模式会看到同一背景和同一轨道几何的
静态版本，信息不依赖运动。代价是系统设置在当前组合存续期间变化不会即时切换，但没有额外后台
监听器，生命周期和性能边界更简单、更可控。

## 验证与边界

- Kotlin 源码审计确认动态分支才创建无限过渡，静态分支只执行一次绘制。
- `token_tracker audit --json` 必须检查新文件头、KDoc、行数和跨端契约引用。
- Android 真机/模拟器的系统“移除动画”设置、帧耗时和 APK 编译仍必须在获批工具链上实测；源码
  通过不替代设备证据。

若未来需要即时响应系统设置，优先增加生命周期感知的 ContentObserver 适配器，并保持该适配器
只更新 UI motion policy；不得把设置读取复制到每个屏幕。

