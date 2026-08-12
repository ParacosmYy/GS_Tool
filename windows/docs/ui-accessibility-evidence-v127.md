# UI 与可访问性证据 v127

## 变更主题

本轮修复最上方 `AI TOKEN / OBSERVATORY` 品牌行看起来像黑色横条的问题。根因是旧个人启动器只加载 `style.css` 与 `ui-polish.css`，其中 header 的纵向 wash 最高 alpha 为 `.34`，叠加背景图后形成明显的黑色分界。

- `ui-polish.css` 将兼容 header 的背景从 `.16/.34/.12` 收敛为 `.07/.16/.035`。
- `brand-transparency.css` 同步完整样式链的最终契约，并将窄屏渐变从 `.42/.18` 收敛为 `.22/.06`。
- 保留 `blur(11px)`、文字 keyline、底部 signal line 与 forced-colors 规则；不引入不透明面板。
- 不改变模板、导航、认证、JavaScript、API、数据库、Provider Key 或数据流。

## 真实运行页面

验证日期：2026-08-12（Asia/Shanghai）。保护端口未重启或停止。

### 桌面首屏

- 地址：`http://127.0.0.1:5000/dashboard?ui_round=127&cache=3`
- 运行窗口：`1683 × 892`
- `.site-header`：`76px` 高，`background-color: rgba(8, 14, 22, 0.07)`。
- `.site-header`：`background-image` 为 `rgba(12, 19, 29, 0.16)` 到 `rgba(8, 14, 22, 0.035)` 的渐变。
- `.site-header`：`backdrop-filter: blur(11px) saturate(1.08)`，不再是深色实底。
- `document.documentElement.scrollWidth=1668`，相对 viewport 没有正向横向溢出。
- 截图确认顶部背景图、导航和 AI TOKEN lockup 连续可见。

### 移动首屏

- 地址：`http://127.0.0.1:5000/dashboard?ui_round=127&cache=mobile`
- 运行窗口：`390 × 844`
- `.site-header`：`344.67px` 内容宽度，背景仍为低 alpha 透景层。
- `AI TOKEN / OBSERVATORY` 品牌文字保持可读，退出按钮未被裁切。
- `document.documentElement.scrollWidth=375`；这是滚动条占用后的布局宽度，没有正向横向溢出。
- 页面控制台 `error/warn=[]`。

### 兼容边界

- `5000` 旧启动器实际加载 `style.css`、`ui-polish.css`，因此本轮将关键修复直接放入 `ui-polish.css`。
- `5011` 现有运行服务返回 `style.css`、`ui-polish.css`、`scene-motion.css`，资源请求均为 `200`；兼容规则同样可由 `ui-polish.css` 覆盖。
- `brand-transparency.css` 作为完整模板链的独立最终契约保留，避免未来入口再次恢复高 alpha header。

## 工程与无障碍边界

- 普通配色继续使用背景图和低 alpha wash；forced-colors 继续恢复 `Canvas`/`CanvasText`，不让透明策略损害系统高对比度。
- reduced-motion 规则未改变；本轮没有新增动画、监听器或交互状态。
- 当前静态行数：`ui-polish.css=867`、`brand-transparency.css=94`、本证据文件 `35` 行，均低于项目 `1000` 行硬门禁。
- 回滚点：仅回滚 `ui-polish.css` 与 `brand-transparency.css` 的 v127 header 背景声明即可恢复 v126 视觉契约。

## 未覆盖门禁

真实设备 UI-3、reduced-motion、高对比度、Android SDK API 37/Build Tools、正式 EXE 签名、真实 Provider usage、HTTPS/ACL、备份恢复与限流演练仍按发布矩阵保持 pending，不能用本轮桌面浏览器证据替代。
