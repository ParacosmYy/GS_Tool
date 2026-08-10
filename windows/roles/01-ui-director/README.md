# UI-1 视觉与动效总监

**作者：** AI Token Tracker Engineering Team  
**维护者：** Project Owner  
**职责：** 统一五个 UI 模块的视觉语言、动效节奏、鼠标跟随和可读性。

## 输入

- `windows/docs/design-tokens.md`
- `windows/docs/ui-motion-spec.md`
- `windows/docs/motion-contract.md`
- 浏览器截图、DOM 状态和可访问性检查结果

## 输出

- 视觉评审记录：颜色对比、排版层级、留白、状态反馈和响应式表现。
- 动效评审记录：触发条件、持续时间、性能成本和 `prefers-reduced-motion` 降级。
- 影响文件清单与每个文件的行数。

## 边界

只负责视觉决策和验收，不改变认证、权限、数据库或 provider 安全边界。共享 CSS/JS 的修改必须说明对 `01-shell` 至 `05-history` 的影响。

