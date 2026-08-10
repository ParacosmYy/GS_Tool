# ADR-090：Rust / RL 嵌入式工程师场景背景 v13

**作者：** AI Token Tracker Engineering Team
**维护者：** UI-1 / UI-2 / UI-3 / ARCH-2
**状态：** Accepted
**日期：** 2026-08-10
**替代：** ADR-086（v12 继续作为回滚资产）

## 背景

v12 已建立成熟御姐气质、嵌入式工程和桌面工作站的视觉方向，但 Rust 底层开发与强化学习构建的叙事仍偏抽象。登录页还需要一张更明确的品牌主视觉：主体应当像真实工程师在工作，而不是普通科技壁纸；左侧标题和认证表单仍必须保持高可读性。

## 决策

1. 新增 v13 像素资产，不覆盖 v12：
   - Web：`windows/token_tracker/static/assets/embedded-rust-engineer-bg-v13.png`
   - Android：`android/app/src/main/res/drawable-nodpi/embedded_rust_engineer_bg_v13.png`
2. Windows 与 Android 使用同一 PNG 内容，默认引用切换到 v13；v12、v11、v10、v9、v8 及更早版本继续保留，作为可回滚历史资产。
3. v13 保持 16:9 横向构图和左侧约 42% 的低细节文字安全区。右侧主体为成年、成熟、可爱但非性化的二次元嵌入式工程师，手持银色 MacBook Pro 风格专业笔记本，工作台放置紧凑银色 Mac Studio 风格桌面工作站。
4. 后方屏幕使用抽象 Rust 代码、固件遥测、强化学习奖励曲线、策略/价值热力图和波形表达工作流；背景不承载产品文案、按钮、密钥、可读业务数据、水印或额外人物。
5. 图片始终是装饰层。Web 使用显式图像层、遮罩和鼠标视差；Android 使用同一资源并由系统减少动画偏好决定静态或低幅漂移。认证卡片、正文、焦点和错误状态的视觉优先级高于背景。

## 资产完整性

- v13 Web/Android SHA-256：`E6F4085853F608BBF1D43FC94ACE17E9EDD8BE1E8EEA162D251AC683BFAB9BB9`
- 资产尺寸：`1672x941`；按 16:9 设计安全区使用。
- 运行时审计：`token_tracker audit --json` 的 `cross-platform-scene`、`contract-references` 和 `web-ui-contracts` 必须通过。
- EXE 必须在默认引用切换后重新构建；旧包不能被描述为包含 v13，直到新的包哈希和隔离启动证据写入后续发布记录。

## 后果与回滚

v13 提升了设备与 Rust/RL 工作流的识别度，并保持登录页左侧文案区域安静。位图体积和真实 Android 裁剪/内存风险仍需设备门禁验证。若真实设备或低端 GPU 出现裁剪、内存或动画问题，将默认引用恢复至 v12 或 v11；不删除 v13，以保留完整回滚链。
