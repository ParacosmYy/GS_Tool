# ADR-086：Rust / RL 嵌入式工程师场景背景 v12

**作者：** AI Token Tracker Engineering Team
**维护者：** UI-1 / UI-2 / UI-3 / ARCH-2
**状态：** Accepted
**日期：** 2026-08-10
**替代：** ADR-080（v11 继续作为回滚资产）

## 背景

v11 已经建立了项目的御姐二次元嵌入式工程师品牌语义，但实际登录首屏复核发现：角色脸部容易被右侧认证卡片切断，笔记本和桌面工作站的层次也不够清晰。背景需要继续美化，同时必须保留网页标题左侧的可读安全区、Android 端的同一品牌资产和低动效策略。

## 决策

1. 新增 v12 像素资产，不覆盖 v11：
   - Web：`windows/token_tracker/static/assets/embedded-rust-engineer-bg-v12.png`
   - Android：`android/app/src/main/res/drawable-nodpi/embedded_rust_engineer_bg_v12.png`
2. 两端使用同一像素文件内容，默认引用切换到 v12；v11、v10、v9、v8 及更早版本继续保留，作为可回滚历史资产。
3. v12 采用 16:9 横向构图：左侧约 38% 保持深海军蓝低细节文案安全区；右侧上方确保成年、成熟且非性化的二次元嵌入式工程师面部完整可见；下方保留银色专业笔记本，最右保留紧凑银色桌面工作站，背景屏幕以抽象 Rust、RL 轨迹和遥测曲线表达工作流。
4. 背景不承载网页 UI、登录卡片、按钮、可读文案或水印；Web 遮罩、认证表单和焦点层继续拥有更高视觉优先级。Android 继续通过系统减少动画偏好决定静态/低幅漂移呈现。

## 资产完整性

- Web/Android v12 SHA-256：`0055F4252E88B068D49A51AFD1AA80D4F768CE48FAEA42284347842007165D89`
- Web 入口：`templates/base.html` 与 `static/scene-motion.css`
- Android 入口：`ui/TokenTrackerApp.kt`
- 运行时审计：`token_tracker audit --json` 的 `cross-platform-scene` 与 `contract-references` 必须同时通过。
- v12 已用当前 checkout 的隔离 Chrome 登录首屏复核；真实设备、浏览器下载落盘、reduced-motion/高对比度和正式发布环境仍属于 UI-3/发布门禁，不在该资产决策中虚构通过。

## 后果与回滚

v12 提升登录首屏的主体识别度和卡片叠加后的脸部可见性，但位图资源体积增加，仍需要在真实 Android 设备上复核内存与裁剪效果。若发现设备裁剪或性能问题，将 Web/Android 默认引用恢复到 v11 或 v10；不删除 v12，以保证回滚路径可审计。
