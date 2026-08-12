# UI Accessibility Evidence v151

日期：2026-08-12

## 变更范围

本轮聚焦 Dashboard 的 `03-observatory / analysis` 表现层。此前趋势图和模型
占比图虽然数据正确，但卡片标题、绘图区与图例之间缺少清晰的扫描边界，窄屏
还会让 doughnut chart 保留 300px 固定上限，可能超出 320px 卡片内容轨道。

现在新增 `token_tracker/static/analysis-signal.css`，由 `base.html` 统一加载：

- 标题轨道增加信号点与底部细线，形成稳定的阅读起点。
- 图表状态增加 `TOKEN SHARE` 胶囊和绘图区内框，不改变 Chart.js 配置或数据。
- 模型图例增加分隔线，标题、图表、图例成为单一路径。
- ≤620px 时 ready chart 使用 `width/min-width/max-width` 收缩契约，避免窄屏
  产生隐藏横向滚动通道。

模板仅为模型占比补充语义性的 `TOKEN SHARE` 状态文本；不改变 API、认证、
数据库、导航或业务数据流。

## 真实浏览器证据

本轮使用项目 `windows/.venv` 启动独立源码验证服务 `localhost:5026`，使用隔离
演示数据库登录。现有 5000/5011 保护服务未重启、未停止、未修改。

| 入口 / 视口 | 场景 | 分析模块 | 横向边界 | 页面日志 |
| --- | --- | --- | --- | --- |
| 5026 / 1440×900 | `ANALYSIS` 深滚 | 双列趋势图/模型占比，标题轨道、状态胶囊和绘图区内框正常 | 文档宽度 1425，无正向溢出 | `[]` |
| 5026 / 390×844 | `ANALYSIS` 深滚 | 单列图表、`TOKEN SHARE` 胶囊不挤压标题，图例保持在卡片内 | 文档宽度 375，无正向溢出 | `[]` |
| 5026 / 320×720 | 首屏源码回归 | 两张 ready chart 均收缩至约 237px 内容轨道，canvas 不越界 | 分析区内部溢出 `0` | `[]` |

桌面端趋势图与环形图均保持 `data-chart-state="ready"`，canvas 仍具备
`role="img"` 及原有中文 `aria-label`。移动端路由上下文和导航
`aria-current="Analysis"` 保持正常。

## 可访问性与降级

- 本轮没有把 canvas 变成新的交互控件，原有图像语义和焦点顺序保持不变。
- `prefers-reduced-motion: reduce` 下装饰光晕关闭，不影响标题、图例或数据阅读。
- `forced-colors: active` 下内框、分隔线、胶囊和信号点统一交给 `CanvasText` /
  `Canvas` 系统色，不依赖固定品牌色。
- `analysis-signal.css` 约 150 行，所有修改文本源码低于 1000 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或
真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
