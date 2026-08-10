# ADR-050：只读发布就绪审计入口

## 状态

已接受（2026-08-10）。

## 背景

项目已经有运行时、Web、Android、EXE 和边缘部署多个交付边界。仅依赖人工记忆和分散命令，容易把
“源码已完成”误报成“APK/EXE/正式 HTTPS 已完成”，也容易在审计时触碰本地数据库或生成敏感产物。

## 决策

- 新增 `python -m token_tracker audit`，只检查关键交付文件、关键契约引用、整个 checkout 中
  非生成代码的 1000 行门禁、企业级作者头、Python 模块公开接口 docstring、Android 公开声明
  KDoc、Windows/Android v7 资产 SHA-256、Python 运行时导入和外部工具链可用性。
- 结果使用稳定的 `pass`、`pending`、`fail` 三态；普通模式允许环境 pending 并返回 0，`--strict`
  将 pending 映射为非零，任何 fail 始终返回非零。
- 审计不调用 `db.init_db`，不读取 `.env` 内容，不创建用户/记录/备份/构建目录，不启动服务，不
  执行 Caddy、Gradle、PyInstaller 或网络请求。
- Android 工具链只有在 JDK、Gradle wrapper/命令、API 37 `android.jar` 和 build-tools
  同时存在时才算 `pass`；只发现 Gradle 时保持 `pending`，避免误报 APK 可构建。
- JSON 输出只包含检查名、状态和脱敏摘要，并使用 ASCII 转义避免 Windows legacy code page 破坏
  重定向文件，便于发布流水线保存证据而不暴露本地绝对路径或凭据。

## 验证边界

- 已完成：CLI help、普通/JSON/strict 三种输出、源码行数和跨端资产检查的本地验证。
- 待完成：正式流水线接入、Android/EXE/Caddy 工具链安装后的 pass 证据以及合法认证会话的浏览器证据。
