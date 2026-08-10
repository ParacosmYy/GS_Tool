# ADR-080：Rust / RL 嵌入式工程师场景背景 v11

**作者：** AI Token Tracker Engineering Team
**维护者：** UI-1 / UI-2 / ARCH-2
**状态：** Accepted
**日期：** 2026-08-10
**作用：** 固化最新 Web 与 Android 共享品牌场景资产的构图、可读性和回滚边界。

## 背景

v10 已能表达嵌入式工程语义，但角色、专业笔记本和紧凑桌面工作站仍需要更清晰的视觉层级。
登录页与仪表盘的左侧是产品正文安全区，右侧才承担品牌叙事；因此需要更成熟、友好、可辨识的二次元
工程师场景，同时不能让背景抢走表单、标题、图表和焦点状态。

## 决策

1. 新增 v11 像素资产，不覆盖 v10；Web 使用
   `windows/token_tracker/static/assets/embedded-rust-engineer-bg-v11.png`，Android 使用同一像素
   内容的 `embedded_rust_engineer_bg_v11.png`。
2. v11 保留 16:9 横向构图：左侧约 42% 是低细节深海军蓝文案安全区，右侧为一名成年、亲和、成熟
   的御姐风嵌入式工程师；她持有银色专业笔记本，旁侧放置独立的银色紧凑桌面工作站，屏幕以
   Rust 底层、RL 轨迹和遥测曲线的抽象图形表达工作流。
3. 设备只使用专业笔记本与紧凑桌面工作站的形态语言，不放置 Apple 或其他品牌 Logo、可读文案和
   水印；品牌语义来自材质、形态和工程环境，不依赖第三方商标。
4. Web 继续由 `scene-motion.css` 管理图片层、对比度遮罩、鼠标视差和低频环境动效；Android
   继续由 Compose 管理图片遮罩和低幅漂移。图片保持 `pointer-events: none`、空替代文本，避免
   进入语义树、抢焦点或承载业务信息。
5. v10、v9、v8 及更早资产继续保留为回滚版本；切换只改变静态资源引用，不改变 API、认证、数据库
   或数据迁移。

## 资产完整性

- Web/Android v11 SHA-256：`48BDB3616CA209F4B6412051F6018C3A47E944D612E966D146EEB4C1DBB7E6A`
- Web 入口：`templates/base.html` 和 `static/scene-motion.css`
- Android 入口：`ui/TokenTrackerApp.kt`
- 运行时审计：`token_tracker audit --json` 的 `cross-platform-scene` 与 `contract-references`
  检查必须同时通过。

## 验证与边界

- 已检查生成图的主体位置、左侧留白、角色完整度、笔记本、桌面工作站、Rust/RL 遥测语义和实验室
  设备；Web/Android 两份 PNG 字节一致。
- 已用静态契约、像素哈希和源码边界验证接入路径；真实 Android 构建、设备视觉验收、系统级
  reduced-motion/高对比度和正式发布环境仍属于独立 UI-3/Android 门禁。

## 回滚

将 Web 模板/CSS 和 Android Compose 资源引用恢复到 v10，即可回滚；不删除 v11，避免再次生成
资产导致跨端像素漂移。
