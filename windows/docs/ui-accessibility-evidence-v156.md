# UI Accessibility Evidence v156

日期：2026-08-12

## 变更范围

本轮只修复 Dashboard 分析区在极窄手机上的标题轨道冲突。`320px` 内容轨道
只有约 `237px`，编号标题与右侧状态胶囊并排时，`01 / VOLUME TREND` 会被
拆成两行，削弱了第一眼的分区识别。本轮在 `analysis-signal.css` 增加
`max-width: 360px` 的局部编排规则：

- `.card-heading` 改为纵向 flex，标题信息与状态胶囊分成两个稳定扫描层。
- eyebrow 使用 `white-space: nowrap`，`01 / VOLUME TREND` 与 `02 / MODEL MIX` 保持单行。
- 状态胶囊左对齐落在标题下方，仍保留颜色、文字和非交互语义；不改变 DOM 或业务状态。
- 390px、桌面和 populated chart 规则不命中该断点。

本轮不改变模板语义、认证、导航、接口、数据库、Provider、Key 或数据流。

## 真实浏览器证据

本轮使用项目 `windows/.venv` 启动独立源码验证服务 `localhost:5028`，使用隔离
演示账户。现有 5000/5011 保护服务未重启、未停止、未修改。

| 入口 / 视口 | 场景 | 关键几何 | 交互 / 横向边界 | 页面日志 |
| --- | --- | --- | --- | --- |
| 5028 / 320×720 `#analysis` | 极窄分析深链 | 目标 `y=84.22`；第一卡 `84.22–475.84`；第二卡 `491.84–890.14`；两个 eyebrow 均单行 `18.15px` | 状态胶囊位于标题下方；两枚空态 CTA `33.48px` 高、`tabIndex=0`；文档宽 `305px` | 应用侧 `[]` |
| 5028 / 390×844 `#analysis` | 普通移动回归 | 标题轨道保持横向；目标 `y=83.93`；第一卡标题 `99.29px` 高 | 规则未命中；空态 CTA `111.33×33.48px`，文档宽 `375px` | 应用侧 `[]` |
| 5028 / 1440×900 `#analysis` | 桌面回归 | 目标 `y=100.71`；第一卡标题 `85.40px` 高；横向状态轨道保持 | 顶栏 `background: transparent`、无 blur/shadow；文档宽 `1425px` | 应用侧 `[]` |

320px 最终截图确认 `01 / VOLUME TREND`、`02 / MODEL MIX` 不再断词，
`输入 + 输出` 与 `TOKEN SHARE` 在各自标题下方形成第二扫描层。分析深链目标
位于透明顶栏底部 `76px` 之后并保留约 `8px` 间距；没有出现横向滚动通道。

## 工程与降级

- 变化限定在既有 `analysis-signal.css` 分析 presentation boundary，无新依赖、脚本、监听器或接口。
- `prefers-reduced-motion` 与 `forced-colors: active` 既有规则继续生效；本轮没有新增动画。
- 规则仅在 `max-width: 360px` 下改变 flex 方向，普通移动和桌面不受影响。
- 修改后的文本源码均低于 1000 行；本轮未创建或运行测试专用资产。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或
真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
