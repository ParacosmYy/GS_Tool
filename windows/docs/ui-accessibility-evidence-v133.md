# UI 与可访问性证据 v133

## 变更主题

本轮处理最上方 `AI TOKEN` 品牌行仍被看成深色横带的问题。真实运行检查显示，旧 `5000` 入口的共享 `ui-polish.css` 仍给 `.site-header` 注入了纵向深色渐变、轻微阴影和 `blur(4px)`；完整样式契约也保留了同一组材质，导致透明场景在顶栏位置出现不连续的黑色观感。

- 共享旧入口与最终品牌契约统一为 `transparent / none / none`：无背景填充、无背景图、无 blur、无阴影、无底边线。
- 文字 keyline、品牌标记 hover/focus、滚动进度线和 forced-colors 系统色边界继续由各自模块负责；没有增加新的 DOM、脚本监听器或业务状态。
- 同步修正 `header-chrome.css`、`scene-motion.css`、`responsive-tuning.css` 的职责注释，避免未来维护者误以为 header 仍应添加 glass wash。

## 真实运行页面

验证日期：2026-08-12（Asia/Shanghai）。保护端口 `5000/5011` 未重启、停止或修改进程。

### 5000 旧个人入口

- `1683×892` 首屏 reload 后，`.site-header` computed style 为：`background-color=rgba(0,0,0,0)`、`background-image=none`、`backdrop-filter=none`、`box-shadow=none`、`border-bottom-color=rgba(0,0,0,0)`，高度 `76px`。
- 截图确认背景插画、角色、设备和 `AI TOKEN` lockup 连续可见，顶栏不再形成黑色覆盖带。
- 深滚至 `scrollY=900` 后，header 获得既有 `is-scrolled` 状态，但材质仍为透明、无 blur、无 shadow；页面宽度保持 `scrollWidth=clientWidth=1668px`。

### 5011 完整运行入口

- `1683×892` reload 后保持相同透明 computed style，完整页面宽度 `scrollWidth=clientWidth=1668px`。
- 截图确认顶部品牌行与背景场景保持连续，Dashboard 首屏构图和导航没有回退。
- 浏览器页面日志为 `[]`；DOM 快照仍包含 `banner`、`main` 和 `AI TOKEN` 品牌链接。

## 工程验证

- `windows/.venv/Scripts/python.exe -m compileall -q windows/token_tracker`：通过。
- `windows/.venv/Scripts/python.exe -m token_tracker audit --json`（`PYTHONPATH=windows`）：`14 pass / 1 pending / 0 fail`。
- `git diff --check`：通过。
- CSS 行数：`ui-polish.css=919`、`brand-transparency.css=100`、`responsive-tuning.css=693`、`scene-motion.css=385`、`header-chrome.css=138`，均低于项目 `1000` 行硬门禁。
- 保护进程保持不变：`5000 → PID 43832`、`5011 → PID 8100`。

## 可访问性与未覆盖门禁

- 原生导航、品牌链接、焦点环、`is-scrolled` 状态、reduced-motion 和 forced-colors 代码边界未改变；本轮未触碰认证、数据、API 或 Provider Key 生命周期。
- 真实设备 UI、reduced-motion 实机、高对比度实机、Android SDK API 37/Build Tools、正式 EXE 签名、真实 Provider usage、HTTPS/ACL、备份恢复与限流演练继续按发布矩阵保持 pending。

## 回滚边界

仅回滚 `ui-polish.css` 和 `brand-transparency.css` 的 header material block，并恢复三个职责注释，即可回到 v132；没有数据库迁移或运行时状态变更。
