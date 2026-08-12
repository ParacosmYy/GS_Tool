# UI 可访问性与视觉证据 · v111

- 主题：登录/注册认证卡片的 signal line 低频呼吸。
- 隔离实例：`127.0.0.1:5212`，数据库位于隔离 runtime；5000/5011 受保护服务未触碰。

## 本轮变更

在普通配色且允许动效时，仅让 `.auth-card::before` 的既有顶部 signal line 追加低频 `auth-card-edge-breathe-v111` 光晕；卡片尺寸、内容、输入焦点、背景图和页面结构不变。`:focus-within` 自动退出追加呼吸，保留原有键盘 focus 边界；forced-colors 与 reduced-motion 继续关闭装饰性动效。

## 浏览器证据

- 桌面 `1440×900`：认证页加载成功，卡片未引入横向溢出；自动聚焦状态下卡片不追加呼吸动画，输入框保留 lime `outline` 与 focus 边界。
- 移动 `390×844`：无自动聚焦时 `auth-card::before` computed animation 包含 `auth-card-edge-breathe-v111`，两次采样的伪元素光晕强度随周期变化；卡片几何约 `344.67×487.98px` 保持稳定，未发生横向溢出。
- 移动可读性：`h1=1`、`h2=1`、8 个有名称的交互控件，`scrollWidth` 未超过 viewport。
- 页面日志：`tab.dev.logs()` 为空；浏览器工具自身 Statsig telemetry 网络告警不属于应用页面日志。

## 仍未覆盖

- 真实设备、forced-colors 实机、Provider 联调、正式生产 WSGI/反向代理部署仍需独立验收。
