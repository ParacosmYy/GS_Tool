# UI 可访问性与视觉证据 · v125

## 变更主题

本轮修复最上方 `AI TOKEN / OBSERVATORY` 品牌行在深色场景中呈现为实色黑条的问题。根因是最后加载的 `brand-transparency.css` 使用 `background: transparent !important` 清除了前面所有材质定义；旧个人启动器只加载基础样式链时也没有统一的顶部阅读层。

现在品牌行使用低 alpha 深蓝玻璃契约：背景插画仍然透出，顶部只保留轻量纵向渐变、`blur(11px)`、弱内侧高光和细边界。`ui-polish.css` 同步提供旧启动器回退，`brand-transparency.css` 作为完整模板链的最终边界；移动规则置于 `forced-colors` 之前，系统高对比模式仍可恢复 Canvas。

## 运行态与工程证据

- 既有本地服务均成功返回新的玻璃样式资源：`127.0.0.1:5000` 的旧入口通过 `ui-polish.css` 兼容层生效，`127.0.0.1:5011` 的完整入口加载最终 `brand-transparency.css`；两者均包含 `backdrop-filter: blur(11px)` 规则。未重启、停止或修改保护服务。
- Python `compileall` 通过，`git diff --check` 通过；没有新增 DOM、API、认证、数据库、Provider Key 或业务数据变更。
- Git 已跟踪文本文件行数门禁通过：所有 `.py/.js/.css/.html/.md/.json/.yml/.yaml/.bat/.ps1/.kt/.java/.gradle` 文件均少于 1000 行。
- 顶部品牌栏的尺寸、导航、Tab 顺序、滚动进度线和 sticky occlusion 行为未改变；只改变视觉材质与兼容样式。

## 角色复核

- UI：顶部不再是连续的黑色覆盖带；角色、MacBook/Mac Studio 与背景光效可透出，AI TOKEN 文本保持清晰。
- 前端：材质边界集中在 `ui-polish.css` 与 `brand-transparency.css`，子级保持透明，不引入脚本监听器或业务状态。
- 后端：无后端、数据库、接口、认证、CSRF、Provider、Key 生命周期或数据流改动。
- 架构师：变化限定在 `01-shell` 视觉契约；`brand-transparency.css` 91 行、`header-chrome.css` 122 行，保留独立回滚边界并覆盖 reduced-motion/forced-colors 降级。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
