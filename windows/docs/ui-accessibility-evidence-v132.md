# UI 与可访问性证据 v132

## 变更主题

本轮聚焦 Dashboard 页面末端：旧 5000 入口的活动历史空态仍保留 populated table 的 `540px` 最小宽度，390px/320px 手机上出现横向滚动条；最近记录空态缺少足够明确的末端信号，页脚则只是弱灰文字和一条横线。

- 仅在 `.activity-history-card.is-empty` 下收缩 table、隐藏表头并允许文案换行；有真实数据时保留原有横向表格契约。
- 页脚加入低 alpha terminal band、局部 lime/lavender signal trace、弱 blur 和更清晰的隐私入口；不增加固定高度，不遮挡背景插画。
- 新增 surface 与 footer 兼容规则均在 `forced-colors: active` 下恢复 Canvas/CanvasText；reduced-motion 与键盘路径不变。

## 真实运行页面

验证日期：2026-08-12（Asia/Shanghai）。保护端口 `5000/5011` 未重启或停止。

### 390px 与 320px 移动端

- 390px 末端：文档宽度 `375px`、横向溢出差值 `-15px`；活动空态 table 宽 `307px`，`overflow-x=visible`，卡片内部溢出 `0`。
- 截图确认“当前还没有工作事件……”完整显示，最近记录空态中心 marker 和文案完整显示，页面底部 signal trace 与隐私入口可见。
- 320px 末端：文档宽度 `305px`、活动空态 table 宽 `237px`、history 宽 `275px`、页脚高度 `118px`，无正向横向溢出。

### 768px 平板与桌面

- 768px 末端：文档宽度 `753px`、活动空态 table 宽 `657px`、卡片内部溢出 `0`，页脚高度 `78px`。
- 1683×892 桌面末端：history 高度 `314px`、活动空态在卡片内保持可读；页脚高 `78px`，背景 `rgba(7,14,24,.02) → .16`，`blur(6px) saturate(1.04)`，局部 trace 宽 `360px`。
- 桌面截图确认末端卡片、背景设备和终止信号带之间有连续层级，没有恢复黑色实面。

### 5011 完整样式链

- `style.css`、`ui-polish.css`、`scene-motion.css` 正常加载；完整链与 5000 旧入口的空态宽度契约一致。
- 5011 浏览器应用日志 `error/warn=[]`；外层浏览器工具偶发 Statsig 网络提示不属于项目页面日志。

## 交互与可访问性

- 活动空态规则只命中 `.is-empty`，不改变 populated rows 的 table width、横向滚动或数据展示。
- `1 个 h1 / 8 个 h2` 结构保持不变；页脚隐私链接仍为原生 anchor，可通过键盘聚焦，颜色和 text-shadow 在普通配色下增强。
- forced-colors 下新增数据面与页脚恢复系统 Canvas/CanvasText，局部装饰 trace 不覆盖用户配色。

## 工程边界

- 修改仅涉及 `legacy-observatory.css`；没有模板、脚本、后端、认证、数据库、Provider Key 或数据流改动。
- 回滚点：移除 v132 empty activity compatibility block 与 terminal footer block，即可回到 v131。
- 文件行数仍低于项目 1000 行硬门禁，兼容逻辑继续集中在旧入口专属模块。

## 未覆盖门禁

真实设备 UI-3、reduced-motion、高对比度实机、Android SDK API 37/Build Tools、正式 EXE 签名、真实 Provider usage、HTTPS/ACL、备份恢复与限流演练仍按发布矩阵保持 pending。
