# ADR-070：嵌入式 Rust/RL 场景背景 v9

**作者：** AI Token Tracker Engineering Team  
**状态：** Accepted  
**日期：** 2026-08-10

## 背景

v8 已完成角色、银色 Pro 形态笔记本、桌面工作站和左侧正文安全区的基础表达。继续视觉验收时，
监控屏上的低层 Rust/RL 工作感、示波器和开发板的层次仍可以更明确，同时角色需要保持“御姐但可爱”
的专业气质。背景属于品牌层，不能通过增加正文、按钮或交互文案来承担业务语义。

## 决策

1. 新增 `embedded-rust-engineer-bg-v9.png`，不覆盖 v8；v8 继续作为可回滚资产。
2. 将同一像素文件复制为 Android 的 `embedded_rust_engineer_bg_v9.png`，并由发布审计比较两端
   SHA-256。当前摘要为 `98E7CD8F291DE1EC2708736B963883CD9C7111CE349F1825C1D40FD8789C54B5`。
3. v9 保持单角色、非性化、成熟可爱的二次元嵌入式工程师构图：右侧放置银色 Pro 形态笔记本、
   紧凑银色桌面工作站、示波器、开发板、服务器和抽象 Rust/RL telemetry；左侧保持深色低细节负空间。
4. Web 显式装饰性 `<img>` 与 CSS `background-image` 回退统一指向 v9；遮罩、pointer follower、
   鼠标视差和 reduced-motion 策略仍属于独立的场景动效层。
5. Android 只替换 drawable 引用，继续由 Compose 控制低幅漂移和遮罩，不复制 Web CSS 动效。

## 验收边界

- 通过：Web/Android 引用统一、PNG SHA-256 一致、生成文件目视检查、左侧正文安全区保留、素材不承载
  可读文案或 logo、源码与文本 1000 行门禁。
- 待完成：真实合法会话下的受保护页面 UI-3 截图、真实 320px 设备指标、Android 编译和设备视觉验收；
  背景升级不替代这些运行时门禁。

## 回滚

将 Web 的 `embedded-rust-engineer-bg-v9.png` 与 Android 的 `embedded_rust_engineer_bg_v9` 引用恢复
到 v8，并同步恢复 `release_audit.py` 的跨端资产检查目标；不删除 v9 文件。
