# ADR-044：Android Release 运行时 HTTPS 边界

## 状态

已接受（2026-08-10）。

## 问题

Gradle Release 配置和 Manifest 已经拒绝明文流量，但如果 endpoint 输入层仍接受
`http://`，用户会在点击登录或执行请求后才看到平台级失败。构建安全门禁和运行时
配置校验必须共享同一环境语义。

## 决策

- `BuildConfig.DEBUG` 是唯一允许明文 HTTP 的 Android 环境开关：Debug 允许模拟器
  `10.0.2.2`，Release 只接受 HTTPS。
- endpoint store、repository 和 `JsonHttpDataSource` 都接收同一个布尔策略；任何一层
  重新配置地址都会在建立连接前调用带策略的规范化函数。
- Release 的旧 Debug HTTP 地址读取失败时回退到构建时的安全默认值；不会尝试发送旧地址。
- Gradle `https` 检查和 Manifest `usesCleartextTraffic=false` 保持不变，形成构建、配置、
  平台三层防线。

## 取舍

- 内部构造器多传一个显式策略参数，但比依赖全局静态状态更容易审计和替换，也避免
  Debug/Release 行为被错误复用。
- 运行时校验不能证明证书可信或主机属于正确的 Windows 部署；这些由 HTTPS 证书、
  反向代理和部署预检负责。

## 验证

- 静态确认 `BuildConfig.DEBUG` 沿 endpoint store → repository → HTTP adapter 单向传播。
- 静态确认 Release 构建仍要求 HTTPS、Manifest 仍禁用明文流量，且没有新增网络依赖。
- 当前机器没有 JDK/Gradle/SDK；APK 编译、安装和真机 HTTPS 联调仍需用户批准工具链。
