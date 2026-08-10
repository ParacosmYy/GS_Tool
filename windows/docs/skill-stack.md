# UI 动效与多端开发技能栈

**作者：** AI Token Tracker Engineering Team  
**来源核对：** `https://github.com/addyosmani/agent-skills`，ref `7676817c12a1317454ae3898a0c5c1eacf5dd3d5`，2026-08-10

这些 skill 是开发代理的工作流，不是应用运行时依赖，因此不复制到 Python 或 APK 的依赖列表中。当前项目按以下顺序使用：

| 技能 | 用途 | 触发范围 |
|---|---|---|
| `frontend-ui-engineering` | 组件、可读性、响应式、WCAG | 五个 UI 模块 |
| `performance-optimization` | 鼠标跟随、动画帧、Core Web Vitals | Shell、Observatory、移动端动画 |
| `browser-testing-with-devtools` | DOM、截图、控制台、网络和性能证据 | 每个网页交付 |
| `code-review-and-quality` | 视觉/状态/边界/可维护性复核 | 每个模块集成前 |
| `source-driven-development` | 框架和 SDK 版本以官方资料为准 | Flask、Compose、Gradle |
| `api-and-interface-design` | 跨网页/Android 的 `/api/v1` 契约 | 认证、日志、统计 |
| `security-and-hardening` | RBAC、密钥、日志、HTTPS、SSRF | 中心化服务 |
| `documentation-and-adrs` | 记录架构取舍和迁移边界 | Windows/Android 分层 |
| `android/skills/adaptive` | Compose 响应式布局资料 | Android 多尺寸屏幕 |
| `android/skills/styles` | Compose 样式与动效资料 | Android 视觉系统（实验 API 需门禁） |
| `android/skills/edge-to-edge` | 系统栏、安全区和 IME 约束 | Android edge-to-edge |

`addyosmani/agent-skills` 提供生命周期工作流；本项目不直接把它当作产品代码依赖。Android AGP 迁移另参考 Android 官方文档及 Kotlin 官方技能包，版本以实际 Android Studio 稳定版为准。

Android 技能资料实际存放在项目内 `android/skills/`，不会写入全局 Codex 配置。当前首条 Android 联动切片只使用已验证的 Compose 基础 API；adaptive/styles 中要求额外导航或实验性 API 的部分留到工具链安装并完成编译验证后再启用。
