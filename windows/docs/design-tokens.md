# Design Tokens

**作者：** AI Token Tracker Engineering Team  
**维护者：** UI-1 / UI-3  
**状态：** Accepted

## 语义层

页面使用 Material 3 风格的语义命名，但保留 Token Tracker 的近黑空间视觉：

| Token | 用途 | 规则 |
|---|---|---|
| `background` | 页面底色 | 近黑，不承载正文大块内容 |
| `surface` | 普通面板 | 与 background 有可辨识层级 |
| `surface-container` | 表单/图表容器 | 维持统一边界和留白 |
| `on-background` / `on-surface` | 主正文和关键数据 | 暗色主题的高对比暖白 |
| `on-surface-variant` | 辅助说明 | 仍须可读，不用“高级灰”牺牲可读性 |
| `outline` / `outline-variant` | 边框、分隔线、focus | 不只靠颜色表达状态 |
| `primary` | 当前操作和焦点 | 全站统一，不在组件内另造 accent |
| `error` | 错误和危险动作 | 同时配文字，不只显示红色 |

## 组件层

组件只能引用语义 token，禁止把页面色值复制到多个选择器。组件层允许根据状态引用同一语义 token：

```css
.provider-card[data-motion-state="error"] {
  border-color: var(--md-sys-color-error);
}
```

## 尺度

- 间距使用 4px 基础尺度；
- 普通触控目标不低于 44px；
- 正文 line-height 不低于 1.5，中文说明优先 1.6–1.8；
- 普通文字目标对比度 4.5:1，大标题 3:1；
- 圆角按层级使用，不给所有元素套同一个大圆角。

## 来源与实现

源文件为 `token_tracker/static/style.css`；如拆分为多个 CSS 模块，`tokens.css` 只能有一份变量真相，其他模块不得重新定义同名 token。
