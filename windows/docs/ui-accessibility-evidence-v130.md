# UI 与可访问性证据 v130

## 变更主题

本轮只处理旧个人体验入口和完整样式链中最上方 `AI TOKEN` 品牌行的材质一致性。上一版共享入口仍使用较重的深色 wash 与 `blur(11px)`，在背景左上角形成黑色横带；本轮将普通配色收敛为近乎清透的轻玻璃窗口。

- 背景色由 `rgba(8, 14, 22, .07)` 降为 `.015`。
- 顶部渐变由 `16% → 3.5%` 降为 `5% → 0%`，移动端增强也同步降为 `8% → 1%`。
- 模糊由 `blur(11px) saturate(1.08)` 降为 `blur(4px) saturate(1.03)`；边界和阴影同步减弱。
- `forced-colors`、`prefers-reduced-motion`、文字 keyline、滚动 signal line 和 DOM/API 契约不变。
- 共享 `ui-polish.css` 与最终 `brand-transparency.css` 同步修改，确保旧 5000 入口无需等待拆分样式链也能生效。

## 真实运行页面

验证日期：2026-08-12（Asia/Shanghai）。保护端口 `5000/5011` 未重启或停止。

### 5000 共享体验入口

- 页面：`http://127.0.0.1:5000/dashboard?ui_round=130_shared&cache=2`
- 桌面视口：`1683 × 892`。
- 实际 computed style：`background-color=rgba(8, 14, 22, 0.016)`、顶部渐变 `rgba(..., 0.05) → rgba(..., 0)`、`backdrop-filter=blur(4px) saturate(1.03)`。
- 页面横向溢出差值为 `-15px`，没有引入新的正向溢出。
- 截图确认背景插画、品牌标记、导航和设备高光在顶部行后保持连续；旧黑色横带不再出现。

### 移动与滚动态

- 移动视口：`390 × 844`；品牌行 `x=15px、width=344.67px、height=76px`，横向溢出差值仍为 `-15px`。
- 滚动态页面：`scrollY=900`，header class 为 `site-header is-scrolled`；computed style 仍为 `blur(4px) saturate(1.03)`、低 alpha 背景和弱阴影。
- 滚动截图确认图表空态、自动采集区和顶部品牌行保持连续阅读层级；既有 CTA 收束与观测区材质没有回退。

### 5011 完整运行链

- 页面：`http://127.0.0.1:5011/dashboard?ui_round=130_full&cache=4`。
- `style.css`、`ui-polish.css`、`scene-motion.css` 正常加载；当前完整运行实例的透明基线仍为 `background=transparent`、`backdrop-filter=none`，没有黑色覆盖材质。
- 浏览器 `error/warn` 应用日志为空；外层浏览器工具偶发的 Statsig 网络提示不属于本地应用日志。

## 工程边界

- 修改仅涉及 `ui-polish.css` 和 `brand-transparency.css`；无模板、脚本、后端、认证、数据库、Provider Key 或数据流改动。
- 视觉职责仍由 `01-shell / brand transparency boundary` 持有；共享入口和完整链使用同一数值契约，避免入口漂移。
- 回滚点：恢复两个样式文件中的 header background、shadow 和 blur 声明即可回到 v129。

## 未覆盖门禁

真实设备 UI-3、reduced-motion、高对比度、Android SDK API 37/Build Tools、正式 EXE 签名、真实 Provider usage、HTTPS/ACL、备份恢复与限流演练仍按发布矩阵保持 pending，不能用本轮桌面浏览器证据替代。
