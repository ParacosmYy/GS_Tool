# UI / Accessibility Evidence · v83

日期：2026-08-12
变更：移除滚动态 AI TOKEN 顶栏的局部 reading rail 与品牌 lens，让整行真正透过场景。
隔离实例：`http://127.0.0.1:5179`
数据：仅使用隔离 SQLite 与合成观察账号；未读取、输出或持久化任何真实 Key、Cookie 或密码。

## 变更边界

- `responsive-tuning.css` 不再为 `.site-header.is-scrolled::before` 绘制右侧深色渐变、blur、边线或阴影。
- `.site-header.is-scrolled .brand::before` 不再绘制包围 `AI TOKEN` 的卡片式透景 lens。
- 顶栏主体仍为 `transparent`、无 `backdrop-filter`、无 `box-shadow`；文字 keyline 与滚动进度线继续保留。
- DOM、导航、认证、API、数据流、业务状态和焦点顺序未改变。

## 实时浏览器证据

| 检查项 | 结果 | 证据 |
| --- | --- | --- |
| 首屏视觉 | pass | 默认桌面视口 `1683 × 892` 截图中，品牌标识直接透过场景，顶部无品牌卡片或整条深色横带。 |
| 滚动态视觉 | pass | `scrollY = 720` 截图中，AI TOKEN、Analysis、Connect、Activity、History 与账户操作继续可读，背景插画连续。 |
| 顶栏 computed style | pass | 首屏与滚动态 `background-color = rgba(0, 0, 0, 0)`、`backdrop-filter = none`、`box-shadow = none`。 |
| 覆盖层 computed style | pass | 滚动态 `site-header::before = display:none`，`brand::before = display:none`；没有局部阅读镜片。 |
| 横向溢出 | pass | `documentElement.clientWidth = 1668`，`scrollWidth = 1668`。 |
| 周期交互 | pass | CUA 点击 `Week` 后活动按钮为 `Week`、状态为“已更新：本周”；再点击 `Today` 后恢复 `Today`、状态为“已更新：今天”。 |
| 可访问性树 | pass | 保留 `banner`、命名导航、`main`、状态区、`统计周期` group、命名周期按钮、表格和表单控件；`AI TOKEN OBSERVATORY` 品牌链接仍可识别。 |
| 页面日志 | pass | `tabV83.dev.logs({})` 返回空日志数组。 |

## 响应式与降级边界

- `min-width: 621px` 的滚动态顶栏伪元素现在明确 `display: none`；`621–900px` 平板覆盖规则同样明确隐藏，不会重新生成深色 reading rail。
- 品牌伪元素隐藏规则不依赖桌面宽度，窄屏滚动也不会恢复卡片 lens。
- forced-colors、reduced-motion、fine-pointer 与既有可访问性规则未被移除。
- 本轮浏览器会话未提供 viewport override 能力，因此没有把默认桌面实时结果扩展成 320/390/768/1024/1440px 的新通过结论；历史矩阵继续参考 v79/v81 证据，真实设备、辅助偏好和正式部署仍保持未关闭。

## 验收结论

v83 的单一 UI 目标已在隔离实例中完成：最上方 `AI TOKEN` 行不再被半透明品牌卡片或滚动态阅读层覆盖，同时保留可读性信号。未改变业务逻辑，也未引入新依赖。
