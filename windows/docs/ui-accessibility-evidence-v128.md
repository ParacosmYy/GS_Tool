# UI 与可访问性证据 v128

## 变更主题

修复旧个人启动器在 Dashboard 滚动后仍保留首屏 CTA 亮色片段的问题。`5000` 只加载 `style.css` 与 `ui-polish.css`，而完整链中的 CTA 收束规则位于 `responsive-tuning.css`，导致旧入口虽然已有 `site-header.is-scrolled` 状态，却没有同步隐藏 `.hero-actions` 和 `.scroll-cue`。

- 将最小兼容规则放入旧入口必加载的 `ui-polish.css`。
- 复用已有 `site-header.is-scrolled` 状态，不新增 JavaScript、监听器、DOM 或接口。
- 首屏 CTA 保持可见；滚动后非焦点 CTA 渐隐、退场并停止指针命中；`:focus-within` 保留键盘可发现性。
- `Scroll to explore` 同步退出 sticky 品牌行阅读带；周期切换器不受影响。

## 真实运行页面

验证日期：2026-08-12（Asia/Shanghai）。保护端口未重启或停止。

### 移动端

- 页面：`http://127.0.0.1:5000/dashboard?ui_round=128&cache=verify`
- 视口：`390 × 844`
- 初始状态：CTA `opacity=1`、`visibility=visible`、`pointer-events=auto`。
- `scrollY=650`：header class 为 `site-header is-scrolled`；CTA `opacity=0`、`visibility=hidden`、`pointer-events=none`、`translateY(-10px)`。
- 同一滚动态：`Scroll to explore` opacity 为 `0`，周期切换器 opacity 为 `1` 且 `pointer-events=auto`。
- 文档宽度 `375`，相对 viewport 没有正向横向溢出。

### 桌面端

- 页面：`http://127.0.0.1:5000/dashboard?ui_round=128&cache=desktop-scroll`
- 视口：`1683 × 892`
- `scrollY=700`：CTA `opacity=0`、`visibility=hidden`、`pointer-events=none`；周期切换器保持可用。
- 横向溢出差值为 `-15px`，没有正向横向溢出。
- 滚动态截图确认亮色 CTA 不再停留在透明品牌行下方，指标轨与图表区成为当前阅读焦点。

### 运行质量

- 页面控制台 `error/warn=[]`。
- 浏览器驱动的合成滚动手势曾出现输入层超时；页面随后通过只读 DOM/计算样式确认已到目标 `scrollY` 和预期收束状态，该工具超时不计为应用错误。
- reduced-motion 下过渡关闭，forced-colors 现有系统配色规则未改变。

## 工程边界

- 修改文件仅为 `ui-polish.css` 与本证据/工程文档；无后端、认证、数据库、Provider Key 或数据流变化。
- 当前 `ui-polish.css` 为 `895` 行，证据文件为 `34` 行，均低于项目 `1000` 行硬门禁。
- 回滚点：移除 `ui-polish.css` 中 v128 compatibility boundary 即可恢复 v127 行为。

## 未覆盖门禁

真实设备 UI-3、reduced-motion、高对比度、Android SDK API 37/Build Tools、正式 EXE 签名、真实 Provider usage、HTTPS/ACL、备份恢复与限流演练仍按发布矩阵保持 pending，不能用本轮桌面浏览器证据替代。
