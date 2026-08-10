# ADR-010：Android 服务端点配置与会话切换

- **状态：** Accepted
- **日期：** 2026-08-10
- **作者：** AI Token Tracker Engineering Team
- **范围：** `android/` 客户端、`/api/v1` 远程适配器

## 背景

Android 模拟器访问宿主机使用 `10.0.2.2`，实体手机和同学分享场景需要使用 Windows 主机的局域网或公网地址。把地址写死在 APK 会造成每次换环境都要重新构建，也容易把本地开发地址误带入发布包。

## 决策

1. 登录页提供 Windows `/api/v1` 服务地址输入框，初始值来自 `BuildConfig.API_BASE_URL`，之后由 `ApiEndpointStore` 保存非敏感地址。
2. 端点只允许 `http`/`https`、主机名和路径；拒绝空白、userinfo、query 和 fragment，长度限制为 300 字符。
3. 用户切换端点时先清除当前加密会话，再配置 HTTP 适配器和新端点；不允许把旧服务 token 发往新服务。
4. API Key、密码、access token 和 refresh token 不进入端点 URL、普通偏好设置、日志或 APK 资源。
5. `http://10.0.2.2:5000/api/v1` 仅用于本机模拟器临时开发；实体设备和公网分享使用 HTTPS，并由部署层负责证书、防火墙、限流和备份。

## 结果

- 同一个 APK 可以连接不同 Windows 中心服务，适合个人本机、局域网和后续公网部署。
- 端点设置可被普通偏好设置读取，访问令牌继续由 Android Keystore 加密容器独立管理。
- 端点切换会要求重新登录，这是有意的安全边界；更换中心服务不会保留上一服务的登录状态。
- 当前机器仍未安装 Android 构建工具链，因此本 ADR 已完成源码与静态边界，APK 编译和真机联调待工具链门禁通过。
