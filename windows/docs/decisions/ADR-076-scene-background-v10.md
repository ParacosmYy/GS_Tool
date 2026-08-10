# ADR-076：Rust / RL 嵌入式工程师场景背景 v10

**作者：** AI Token Tracker Engineering Team  
**维护者：** UI-1 / UI-2 / ARCH-2  
**状态：** Accepted  
**日期：** 2026-08-10  
**作用：** 固化 Web 与 Android 共享的品牌场景资产、构图安全区和回滚边界。

## 背景

v9 已解决“右侧场景过暗、用户看不懂产品主题”的问题，但笔记本、桌面主机和屏幕内容仍偏
泛化，无法一眼表达“成熟二次元嵌入式工程师正在用 Rust 做底层 RL 学习构建”。本次需求要求
继续美化背景，同时不能牺牲登录页左侧标题、表单和移动端内容的可读性。

## 决策

1. 新增 v10 像素资产，不覆盖 v9；Web 使用
   `windows/token_tracker/static/assets/embedded-rust-engineer-bg-v10.png`，Android 使用同一像素
   内容的 `embedded_rust_engineer_bg_v10.png`。
2. v10 保留 16:9 横向构图：左侧约 42% 为低细节深色文案安全区，右侧为一名成年、成熟且亲和的
   二次元嵌入式工程师；她手持银色专业笔记本，屏幕以 Rust 代码、训练曲线和观测图形表达工作
   语义，右下角为银色紧凑桌面工作站，并保留示波器、开发板、服务器和线缆。
3. 设备采用当前专业笔记本/紧凑桌面工作站的外形语言，不放置 Apple 或其他品牌 Logo、可读营销
   文案或水印；品牌识别来自材质、形态和工作流，而不是第三方商标。
4. Web 继续由 `scene-motion.css` 管理滤镜、遮罩、鼠标视差和低频环境动效；Android 继续由
   Compose 管理遮罩和低幅漂移。图片层保持 `pointer-events: none`，不进入语义树、不抢焦点。
5. v9、v8 及更早资产保留作为可回滚版本；切换只改变静态资源引用，不改变 API、认证、数据库
   或数据迁移。

## 资产完整性

- Web/Android v10 SHA-256：`04DBB4FD9F57CBAAA9D5D719D6B562AD4F22B13AF87691638B6B370E8AB2FB7E`
- Web 入口：`templates/base.html` 和 `static/scene-motion.css`
- Android 入口：`ui/TokenTrackerApp.kt`
- 运行时审计：`token_tracker audit --json` 的 `cross-platform-scene` 与 `contract-references`
  检查必须同时通过。

## 验证与边界

- 已检查生成图的主体位置、左侧留白、笔记本/RL 屏幕、银色桌面工作站、示波器和开发板；两端
  文件字节一致。
- 已用静态契约、编译诊断和资源哈希验证接入路径；真实 Android 构建、设备视觉验收、系统级
  reduced-motion/高对比度和正式发布环境仍属于独立 UI-3/Android 门禁。

## 回滚

将 Web 模板/CSS 和 Android Compose 资源引用恢复到 v9，即可回滚；不删除 v10，避免再次生成
资产导致跨端像素漂移。
