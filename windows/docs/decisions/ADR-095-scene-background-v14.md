# ADR-095：Rust / RL 嵌入式工程师场景背景 v14

**作者：** AI Token Tracker Engineering Team
**维护者：** UI-1 / UI-2 / UI-3 / ARCH-2
**状态：** Accepted
**日期：** 2026-08-10
**替代：** ADR-090（v13 继续作为回滚资产）

## 背景

v13 已经建立角色、设备和 Rust/RL 工作流的识别度，但人物轮廓、专业设备层次和实验室光影在登录首屏仍有继续收束空间。用户要求背景继续向“成年御姐气质、可爱但专业的二次元嵌入式工程师”方向提升，并明确表现 MacBook Pro / Mac Studio 风格设备与 Rust 底层强化学习构建。

## 决策

1. 新增 v14 像素资产，不覆盖 v13：
   - Web：`windows/token_tracker/static/assets/embedded-rust-engineer-bg-v14.png`
   - Android：`android/app/src/main/res/drawable-nodpi/embedded_rust_engineer_bg_v14.png`
2. Windows 与 Android 使用同一 PNG 内容，默认引用切换到 v14；v13、v12、v11、v10、v9、v8 及更早版本继续保留，作为可回滚历史资产。
3. v14 保持 16:9 横向构图和左侧约 42% 的低细节文字安全区。右侧主体为成年、成熟、可爱但非性化的二次元嵌入式工程师，手持银色 MacBook Pro 风格专业笔记本，工作台放置紧凑银色 Mac Studio 风格桌面工作站。
4. 后方屏幕使用抽象 Rust 代码、奖励曲线、策略/价值可视化、波形与嵌入式遥测表达工作流；背景不承载产品文案、按钮、密钥、可读业务数据、水印或额外人物。
5. 图片始终是装饰层。Web 使用显式图像层、遮罩和鼠标视差；Android 使用同一资源并由系统减少动画偏好决定静态或低幅漂移。认证卡片、正文、焦点和错误状态的视觉优先级高于背景。

## 资产完整性

- v14 Web/Android SHA-256：`E30A3B402D4BA0312A20CE8DE44F6933C925152D46CE72FC5FE02249BD06FDF8`
- 资产尺寸：`1672x941`；文件大小 `1,660,273` bytes；按 16:9 设计安全区使用。
- 生成方式：项目视觉资产生成流程；生成源保留在本机 Codex 资产目录，项目只提交最终运行时 PNG。
- 运行时审计：`token_tracker audit --json` 的 `cross-platform-scene`、`contract-references` 和 `web-ui-contracts` 必须通过。
- EXE 必须在默认引用切换后重新构建；旧包不能被描述为包含 v14，直到新的包哈希和隔离启动证据写入后续发布记录。

## 后果与回滚

v14 提升了人物、笔记本、桌面工作站和 Rust/RL 实验屏幕的可辨识度，同时继续保护登录页左侧正文。位图体积和真实 Android 裁剪/内存风险仍需设备门禁验证。若真实设备或低端 GPU 出现裁剪、内存或动画问题，将默认引用恢复至 v13 或 v12；不删除 v14，以保留完整回滚链。
