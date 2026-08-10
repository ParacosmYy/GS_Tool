# ADR-027：升级跨端嵌入式 Rust/RL 品牌背景 v4

**状态：** Accepted  
**日期：** 2026-08-10  
**范围：** Windows Web、Android Compose、`static/assets/`、Android `drawable-nodpi`

## 背景

现有 v3 背景已经建立了嵌入式实验室语义，但登录页在不同断点下仍需要更明确的角色叙事和设备关联。用户希望首屏表达“嵌入式工程师 + Rust 底层 + RL 学习构建”，同时保持左侧标题和表单的高可读留白。

## 决策

1. 生成独立的 `embedded-rust-engineer-bg-v4.png`，不覆盖 v3，以保留可回滚资产。
2. 画面使用一名成年二次元嵌入式工程师、银色 Pro 笔记本、桌面工作站、实验板和抽象 RL 曲线；品牌标识与可读文字不嵌入图片，避免资产承担业务信息。
3. Web 通过 `.story-backdrop` 统一引用 v4；背景层保持 `pointer-events: none`，由遮罩、正文和交互层拥有更高视觉优先级。
4. Android 同步复制同一 PNG，并仅替换 Compose 背景资源引用；动效和渐变仍由端侧代码独立控制，避免跨端耦合布局实现。
5. 资产不包含用户数据、密钥、日志或远程运行时依赖；动画只对背景做低幅漂移，并继续遵守 `prefers-reduced-motion`/端侧降级规则。

## 验收证据

- Web 真实 Chrome 截图已生成：`windows/.cache/ui-audit-v4-1440.png`、`ui-audit-v4-1024.png`、`ui-audit-v4-768.png`。
- 1440px 下左侧大标题、中文说明和右侧登录卡片保持可读；768px 下登录卡片完整落入视口，背景不会进入命中区域。
- v4 Windows/Android 资产尺寸和字节内容一致；Android APK 尚未在用户批准工具链前构建，不能据此宣称 APK 产物验收通过。

## 取舍与后续

- 桌面端人物会被表单卡片局部遮挡，这是为了保证表单不依赖图片对比度；如后续需要完整人物海报，应另设不承载表单的 landing route，不削弱认证页遮罩。
- v3 及旧版本保留为回滚参考，不参与默认渲染。
