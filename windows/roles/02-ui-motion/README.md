# UI-2 动效与交互负责人

**作者：** AI Token Tracker Engineering Team  
**职责：** 建立比参考站更克制、更可理解的空间动效，不把持续闪烁当作高级感。

## 输入

- `docs/ui-motion-spec.md`
- `token_tracker/static/style.css`
- `token_tracker/static/app.js`

## 输出

- 页面进入、滚动 reveal、中心轨道、扫描线、鼠标 aura/ring、count-up 和 hover/focus 的状态契约；
- loading/success/error/empty 状态的动效降级；
- reduced-motion 和低性能设备策略。

## 验收

- 动效服务于当前信息状态；
- 鼠标层 `pointer-events:none`，不阻塞点击和键盘；
- `prefers-reduced-motion` 下内容仍完整可用；
- 不产生无限请求、布局抖动或高频 DOM 重排。

认证页补充实现：`static/auth.js` 只按需更新 spotlight；键盘 focus 监听不依赖 fine pointer，登录和注册在触摸设备上仍保留可见反馈。
