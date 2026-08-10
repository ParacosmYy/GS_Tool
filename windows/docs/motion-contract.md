# Motion Contract

**作者：** AI Token Tracker Engineering Team  
**维护者：** UI-2 / UI-3 / 前端工程师  
**状态：** Accepted

动效是信息状态的表达层，不是独立装饰层。所有持续动画都必须可以在没有动画时理解。

## 状态集合

```text
idle → loading → success
  │       │
  └───────┴────→ error

empty 是数据状态，不是请求状态。
reduced 是用户偏好覆盖层，覆盖所有非必要运动。
```

| 状态 | 语义 | 视觉反馈 | 语义反馈 |
|---|---|---|---|
| `idle` | 尚未动作 | 静态面板、低频轨道 | 无需播报 |
| `loading` | 请求处理中 | 轻微扫描/按钮禁用/骨架 | `aria-busy="true"` |
| `success` | 请求完成 | 数字 count-up、一次性 sheen | `role=status` 成功文本 |
| `error` | 请求失败 | 边框和图标变更，禁止抖屏 | `role=status` 错误文本 |
| `empty` | 范围无数据 | 保留结构，显示说明和下一步 | 可读空状态 |
| `reduced` | 用户关闭运动 | 去除跟随、轨道循环和 reveal 位移 | 内容顺序不改变 |

## DOM 状态约定

- 可异步操作的容器使用 `data-motion-state="idle|loading|success|error|empty"`。
- 提交中的表单设置 `aria-busy="true"`，完成后恢复 `false`。
- 成功/失败文案放在同一 `role="status" aria-live="polite"` 节点，不能只改变颜色。
- 禁用按钮是状态结果，不是唯一 loading 反馈；文本必须说明正在发生什么。
- `prefers-reduced-motion: reduce` 由 CSS 和 JS 同时处理，JS 不启动 pointer follower 和无限 requestAnimationFrame 循环。

认证页使用同一套约束：`.auth-shell` 负责入场与信号场，`.auth-orbit-art` 负责低频轨道，`auth.js` 只在 fine pointer 下更新 spotlight；焦点反馈独立于指针能力，触摸和键盘用户仍能看到表单状态。

## 节奏基线

- page enter：700–1100ms；
- reveal：350–1000ms，按信息层级错峰；
- hover/focus：180–350ms；
- count-up：650–950ms，始终以服务端最终值收敛；
- 永久循环只保留低频轨道/呼吸/扫描，禁止全文持续 glitch。

## 性能边界

- 动画优先 `transform` 和 `opacity`，不在每帧读取布局；
- pointer follower 必须 `pointer-events:none`，且只在 fine pointer 启用；
- 图表和记录列表是数据真相，动效失败不能阻塞它们渲染；
- 任何新增动画都必须说明 reduced-motion 降级和移除条件。
