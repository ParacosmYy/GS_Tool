# ADR-057：场景主图显式图像层与确定性合成

**作者：** AI Token Tracker Engineering Team
**维护者：** Project Owner
**状态：** Accepted
**日期：** 2026-08-10

## 背景

v7 场景资产已经满足品牌构图，但部分 Chromium 渲染路径对“固定空元素 + CSS
`background-image` + 负层级”合成不稳定：样式、网格和遮罩正常出现时，主角色图片可能
在首屏被画布背景吞掉。这会让用户误以为背景没有加载，也无法满足直接打开即可体验的要求。

## 决策

- `base.html` 使用显式的装饰性 `<img>` 作为场景主图，`alt=""` 且父容器
  `aria-hidden="true"`，不进入语义树、焦点顺序或业务状态。
- `.story-backdrop` 仍保留 CSS `background-image` 作为资源失败时的回退；主图通过
  `object-fit: cover`、响应式 `object-position` 和有限像素平移实现桌面/移动端构图。
- 采用明确的层级契约：场景为 `z-index: 0`，主内容和页脚为 `z-index: 1`，站点导航和
  pointer affordance 保持更高层级。场景始终 `pointer-events: none`，不能遮挡交互。
- 背景视差只更新主图 transform 变量；遮罩、光束和 `prefers-reduced-motion` 规则继续
 由场景 CSS 管理，不把业务状态耦合到插画层。

## 验证边界

- 已完成：当前源码进程在本机 Chrome 生成 1440、768、320 宽度截图；角色、笔记本、
  Mac Studio 风格工作站和开发板可见，登录正文、输入焦点和卡片层保持可读。
- 已完成：入口 HTML 仍保持装饰层 `aria-hidden`，主图不产生横向溢出，前端脚本语法和
  只读发布审计继续作为提交门禁。
- 待完成：合法认证会话下的 dashboard/admin/connection 四档证据，以及 Android
  工具链批准后的端侧视觉验收。

## 回滚

保留 `.story-backdrop` 的 CSS 背景声明和 v6/v7 资产。若显式图像层在目标浏览器出现
兼容性问题，可暂时移除 `<img>`，恢复仅 CSS 背景渲染；不得删除 v7 或 v6 回滚资产。
