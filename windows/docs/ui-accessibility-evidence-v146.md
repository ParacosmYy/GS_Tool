# UI Accessibility Evidence v146

日期：2026-08-12

## 变更范围

本轮收口 Dashboard `05 / WORK SIGNAL` 工作信号录入区。原先项目、任务类型、正确码、错误码分散在主表单中，窄屏会形成连续长表单；本轮新增语义化 `fieldset`，将四个可选字段归入“补充信息”分组：390px 及以上保持两列元数据轨道，≤340px 自动降为单列，确保示例文本与输入目标不被压缩。

新增 `activity-signal.css` 作为 `05-activity / capture surface` 专属表现模块，并由共享 `ui-polish.css` import。模板仅重组已有字段，不改变字段 name、提交脚本、API 契约、数据库映射或数据流。

## 真实浏览器证据

本轮使用项目 `windows/.venv` 启动独立源码验证服务 `127.0.0.1:5026`，避免触碰受保护的既有 5000/5011 服务。验证完成后临时服务关闭；保护服务 PID 未重启、未停止、未修改。

| 视口 | 录入卡片高度 | 补充信息高度 | 元数据列 | 提交按钮 | 文档溢出 | 页面日志 |
| --- | ---: | ---: | --- | --- | --- | --- |
| 320×780 | `1150px` | `380px` | 单列 `220px` | 未进入首屏但保持 48px | 无 | `[]` |
| 390×844 | `967px` | `213px` | 双列 `136px + 136px` | `307.3 × 48px` | 无 | `[]` |
| 1440×900 | `838px` | `213px` | 双列 `305.74px + 305.75px` | 原桌面主动作几何保持 | 无 | `[]` |

390px 回归中，保存按钮从旧布局约 1143px 的页面坐标上移至约 951px，主路径减少约 145px 的无效滚动；320px 进入单列降级后，四个示例输入宽度为 `220px`、高度为 `52px`，可读性优先于压缩高度。

## 可访问性与降级

- 新增 `fieldset` / `legend` 为补充字段提供语义分组；字段 name 与 DOM 内焦点顺序保持稳定：`direction`、`outcome`、`efficiency_score`、`project`、`task_type`、`result_code`、`error_code`、`note`。
- 390px 与桌面使用双列仅改变视觉布局，不隐藏字段；≤340px 单列降级避免占位文本被裁切。
- 所有表单控件保持 `tabIndex=0`；提交按钮保持 `48px` 高度与现有键盘焦点契约。
- `forced-colors: active` 恢复 `CanvasText` / `Canvas` 边界；`prefers-reduced-motion: reduce` 不引入新的持续动画。
- 修改后文件仍低于 1000 行：`activity-signal.css` 122 行，`ui-polish.css` 990 行，`dashboard.html` 229 行，`style.css` 674 行，`responsive-tuning.css` 693 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
