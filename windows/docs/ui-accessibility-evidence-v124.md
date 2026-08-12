# UI 可访问性与视觉证据 · v124

## 变更主题

本轮稳定 Dashboard 分享链接的初始 hash 定位。图表、记录和工作事件在首个文档 paint 后异步加载，原生 hash 位置可能基于旧高度计算，导致 `#connect`、`#activity`、`#history` 分享链接落在错误章节。现在导航模块仅在初始 hash 存在时启动一个有时限的校正器：使用即时滚动重新对齐目标，连续稳定后释放；用户开始滚轮、触摸、指针或键盘操作时立即停止，普通导航仍保留既有平滑滚动。

用户服务：`127.0.0.1:5000`。验证期间未重启或修改受保护的 `5000/5011` 服务，也未改变数据库、认证、Provider Key 或业务数据。

## 浏览器证据

- 使用全新浏览器页面上下文，在 `390×844` 下分别打开 `#connect`、`#activity`、`#history` 分享链接；异步页面稳定后，三个目标章节均落在透明顶栏底部约 `76px` 以下，章节标题顶部约 `132px`，标题完全可读。
- 干净上下文矩阵最终结果：`#connect` `sectionTop≈103.8px`、`headingTop≈132.4px`；`#activity` `sectionTop≈103.7px`、`headingTop≈132.4px`；`#history` `sectionTop≈334.5px`、`headingTop≈360.1px`，均没有被顶栏遮挡。
- 初始校正阶段 `document.documentElement.scrollBehavior` 为 `auto`，稳定后恢复 `smooth`；`#connect` 初始校正完成后 `scroll-motion-enabled=true`，不会长期接管用户滚动。
- 普通无 hash Dashboard 首屏保持 `scrollY=0` 且平滑滚动启用；点击导航 `Connect` 后仍能平滑到达，最终 `headingTop≈132.4px`、hash 为 `#connect`。
- 页面没有正向横向溢出，`document.documentElement.scrollWidth - innerWidth = -15px`，差值来自浏览器滚动条占位；应用页面日志为空。
- 页面既有 `1 个 h1 / 8 个 h2`、焦点样式、透明顶栏和 sticky occlusion 契约不变。

## 工程与无障碍协议

- 变化限定在 `modules/navigation.js` 与基础 `style.css`：不新增 DOM、依赖、API、认证逻辑或数据状态。
- 校正器最多运行约 `2.8s`，连续三个稳定帧后提前释放；用户输入事件优先级高于自动校正，避免抢夺滚动控制。
- `html.scroll-motion-enabled` 只在初始化 hash 处理完成或页面无 hash 时启用；`prefers-reduced-motion` 仍由既有系统规则覆盖为即时滚动。
- `style.css` 共 `674` 行，`navigation.js` 共 `290` 行，均未超过项目每文件 1000 行约束。

## 角色复核

- UI：分享链接打开后直接进入正确的“观测章节”，标题与表单上下文连续，不再出现先落错位置再回弹的视觉抖动。
- 前端：定位稳定器归属导航展示层，复用现有 `scrollMarginTop`、ResizeObserver 和锚点契约，不触碰业务 API。
- 后端：无后端、数据库、认证、Provider、Key 生命周期或数据流改动；保护端口 `5000/5011` 未触碰。
- 架构师：变化限定在 `01-shell / navigation contract`，自动校正有明确时限与取消边界，无新依赖、无跨层耦合，回滚边界为 v124 导航函数和滚动 class。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
