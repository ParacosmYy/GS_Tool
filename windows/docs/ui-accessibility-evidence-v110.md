# UI 可访问性与视觉证据 · v110

- 主题：AI TOKEN 顶部品牌行透光层收敛。
- 隔离实例：`127.0.0.1:5211`，使用项目 `.venv`，数据库位于隔离 runtime；未触碰受保护的 5000/5011 服务。
- 真实页面：`/login` 与登录后的 `/dashboard`。

## 本轮变更

`responsive-tuning.css` 将普通配色下的 `.site-header` 与 `.site-header.is-scrolled` 统一为低 alpha 垂直渐变，关闭 `backdrop-filter` 与深色外投影，仅保留极轻的底部内侧高光；forced-colors 分支继续由系统 Canvas 颜色接管。

## 浏览器证据

- 桌面登录页：computed `background-color: transparent`、`backdrop-filter: none`；顶部品牌行不再形成模糊横带。
- 桌面 Dashboard：`1440×900`，首屏 `scrollY=0`；顶部品牌行与插画连续透景，`scrollWidth` 未超过 viewport，`h1=1`。
- 桌面深滚：真实设置 `scrollY=1200` 后 header class 为 `site-header is-scrolled`，computed `backdrop-filter: none`、无深色外投影，页面无横向溢出。
- 响应式：`390×844` 与 `320×720` 均 computed `backdrop-filter: none`，header 高度 `76px`，`document.body.scrollWidth` 分别为 `375/305`，未发生横向溢出。
- 页面日志：`tab.dev.logs()` 为空。浏览器工具自身的 Statsig telemetry warning 不属于应用页面日志。

## 仍未覆盖

- 真实设备、forced-colors 实机、Provider 联调、正式生产 WSGI/反向代理部署仍需独立验收。
- 本证据只覆盖视觉层与响应式结构，不代表生产安全或第三方 API 可用性。
