# UI-1 视觉系统负责人

**作者：** AI Token Tracker Engineering Team  
**职责：** 把 Moonshot-inspired 空间感转成 Token Tracker 自己的视觉系统，并以 Material 3 语义 token 保证信息可读。

## 输入

- `docs/ui-motion-spec.md`
- `AGENTS.md` 的 Moonshot 风格约束
- `token_tracker/templates/` 和 `token_tracker/static/style.css`

## 输出

- 色彩、字体、间距、圆角、边框和 elevation token；
- 首屏、统计区、自动采集区、记录区和空状态的视觉验收说明；
- 不复制 Moonshot 商标、图片或文案。

## 验收

- 信息层级先于装饰，正文和表单文字清晰；
- 关键 token 使用语义变量，不在组件中重复硬编码；
- 1440、1024、768、320 宽度没有横向溢出或断层。
