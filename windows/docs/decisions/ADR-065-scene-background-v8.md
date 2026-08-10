# ADR-065：登录场景背景 v8 视觉升级与跨端一致性

**作者：** AI Token Tracker Engineering Team
**状态：** Accepted
**日期：** 2026-08-10

## 背景

v7 已经建立了右侧工程师、Pro 笔记本、桌面工作站和左侧文案安全区，但在实际登录首屏中，
角色面部、设备轮廓和实验台的层次仍偏暗。用户希望背景更明确地表达“御姐二次元嵌入式工程师
使用 Rust 进行底层 RL 学习构建”的产品气质，同时不能降低表单和标题的可读性。

## 决策

1. 生成独立的 `embedded-rust-engineer-bg-v8.png`，不覆盖 v7，以保留可回滚资产。
2. 将同一像素文件复制为 Android 的 `embedded_rust_engineer_bg_v8.png`，并由发布审计比较两端
   SHA-256；当前摘要为 `D85B08787BEC88B558E32BE99921F327EC099B3D2B550B0773143D2C7DF3BE56`。
3. v8 继续使用单角色、非性化的成熟可爱二次元工程师构图：右侧放置银色 Pro 笔记本、桌面
   Mac Studio 风格计算机、示波器和实验板，左侧保持深色低细节负空间。
4. Web 仍由显式装饰性 `<img>` 作为主图、CSS `background-image` 作为回退；遮罩、pointer
   follower、鼠标视差和 reduced-motion 降级策略不与素材版本耦合。
5. Android 只替换 drawable 引用，继续由 Compose 自己控制低幅漂移和遮罩；不把 Web CSS 动效
   复制到 Android。

## 验收边界

- 通过：Web/Android 引用统一、PNG SHA-256 一致、生成文件目视检查、素材不承载可读文案或 logo、
  左侧安全区保留、源码与文本 1000 行门禁。
- 待完成：真实合法会话下的受保护页面 UI-3 截图、真实 320px 设备指标、Android 编译和设备视觉
  验收；背景升级不替代这些运行时门禁。

## 回滚

将 Web 的 `embedded-rust-engineer-bg-v8.png` 与 Android 的 `embedded_rust_engineer_bg_v8`
引用恢复到 v7，并同步恢复 `release_audit.py` 的跨端资产检查目标；不删除 v8 文件。
