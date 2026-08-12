# UI 与可访问性证据 v129

## 变更主题

本轮收敛 Dashboard 观测区的材质断层：旧入口的周期切换器原本是偏重的深色胶囊，空态图表卡片则接近不透明黑面；两者都与透明指标轨和背景插画割裂。改动将观测区统一为低 alpha 深蓝信号窗口，同时补齐旧入口缺失的空态网格、局部标题线和文字 keyline。

- 周期切换器使用低 alpha 深蓝渐变、轻微内侧高光和清晰边界；active/hover/focus 语义不变。
- 图表卡片使用低 alpha 深蓝渐变、轻量 `blur(8px) saturate(1.04)` 和轻阴影；不恢复黑色实面。
- 空态图表增加 `NO SIGNAL / READY`、网格、局部 signal line、文字 keyline 和阅读底；不改变 Chart.js、接口或空态文案契约。
- `ui-polish.css` 通过 `@import` 加载独立 `legacy-observatory.css` 兼容旧 `5000` 入口；`observatory-signal.css` 继续拥有完整链的 populated chart material。

## 真实运行页面

验证日期：2026-08-12（Asia/Shanghai）。保护端口未重启或停止。

### 桌面首屏

- 页面：`http://127.0.0.1:5000/dashboard?ui_round=129&cache=desktop`
- 视口：`1683 × 892`
- 周期切换器实际计算为低 alpha 深蓝渐变，边界 `rgba(230, 235, 245, .34)`，无 blur。
- 空态图表卡片实际计算为深蓝渐变，边界 `rgba(230, 235, 245, .24)`，无正向横向溢出。
- 截图确认首屏构图和 v128 CTA 收束行为未回退。

### 移动首屏与深滚

- 页面：`http://127.0.0.1:5000/dashboard?ui_round=129&cache=split`
- 视口：`390 × 844`，文档横向溢出差值 `-15px`。
- 图表卡片 `backdrop-filter: blur(8px) saturate(1.04)`；空态图表保留网格和 `NO SIGNAL / READY`。
- `scrollY=1000` 深滚时 header 为 `site-header is-scrolled`，周期切换器仍为 `pointer-events=auto`。
- 深滚截图确认指标轨、图表标题、空态 marker、说明文字和自动采集入口具有连续阅读层级，背景设备仍可见。

### 交互与质量

- 键盘将周期切换器切换至 Week 后，active period 为 `week`，`aria-pressed` 为 `[false,true,false,false]`。
- 桌面与移动页面控制台 `error/warn=[]`。
- forced-colors 下观测卡片仍由系统 Canvas 接管，reduced-motion 现有规则未改变。

## 工程边界

- 修改涉及 `ui-polish.css`、新增 `legacy-observatory.css`、`observatory-signal.css` 与文档；无后端、认证、数据库、Provider Key 或数据流改动。
- 当前行数：`ui-polish.css=872`、`legacy-observatory.css=131`、`observatory-signal.css=196`、本证据文件 `39` 行，全部低于项目 `1000` 行硬门禁。
- 回滚点：移除 v129 的观测区兼容块和 `observatory-signal.css` populated chart block，即可恢复 v128 材质。

## 未覆盖门禁

真实设备 UI-3、reduced-motion、高对比度、Android SDK API 37/Build Tools、正式 EXE 签名、真实 Provider usage、HTTPS/ACL、备份恢复与限流演练仍按发布矩阵保持 pending，不能用本轮桌面浏览器证据替代。
