# UI Accessibility Evidence v148

日期：2026-08-12

## 变更范围

本轮修复最上方 `AI TOKEN` 品牌行的“透明但像有黑色横带”观感。根因不是
`background` 填充，而是兼容样式链中的低 alpha inset shadow：它会在明亮
插画上合成一条材质带。`brand-clarity.css` 现将普通配色下的共享 header
收敛为 `transparent / no shadow / no blur`；`brand-transparency.css` 继续
作为完整样式链的最终兜底。signal line、文字 keyline、focus ring 和系统色
降级保持不变。

本轮不改变模板、导航语义、认证、数据库、Provider、API、Key 生命周期或
业务数据流。

## 真实浏览器证据

本轮使用项目 `windows/.venv` 启动独立源码验证服务 `localhost:5026`，并读取
现有 `5000` / `5011` 入口的实际页面计算样式。保护服务未重启、未停止、未
修改。浏览器验证只进行了导航、视口切换、DOM/计算样式读取和截图。

| 入口 / 视口 | 页面 | header 宽度 | `background` | `box-shadow` | `backdrop-filter` | 文档溢出 | 页面日志 |
| --- | --- | ---: | --- | --- | --- | --- | --- |
| 5026 / 1440×900 | 登录 | `1320px` | transparent | none | none | 无 | `[]` |
| 5026 / 390×844 | 登录 | `344.67px` | transparent | none | none | 无 | `[]` |
| 5026 / 1440×900 | 注册 | `1320px` | transparent | none | none | 无 | `[]` |
| 5026 / 390×844 | 注册 | `344.67px` | transparent | none | none | 无 | `[]` |
| 5000 / 1440×900 | 登录重定向页 | `1320px` | transparent | none | none | 无 | `[]` |
| 5011 / 1440×900 | 登录重定向页 | `1320px` | transparent | none | none | 无 | `[]` |
| 5000 / 390×844 | 登录重定向页 | `344.67px` | transparent | none | none | 无 | `[]` |

隔离登录页的桌面卡片保持 `440px` 宽，移动卡片保持 `344.67px` 宽；注册页
桌面双栏卡片保持 `596px` 宽。移动端登录页的表单入口仍在首屏内，顶部品牌
行与场景插画连续透景，没有产生新的横向滚动通道。

## 可访问性与降级

- header 仍是语义化 sticky 容器，品牌链接、表单字段、主按钮和注册链接的
  Tab 顺序未改变。
- 透明契约只移除视觉阴影，不移除 `:focus-visible`、键盘焦点或文本 keyline。
- `prefers-reduced-motion: reduce` 与 `forced-colors: active` 规则保持有效；
  系统色模式仍由 `Canvas` / `CanvasText` 接管背景与边界。
- signal line 是独立伪元素，不承载交互和语义；焦点反馈不依赖动画或鼠标。
- 修改后 `brand-clarity.css`、`brand-transparency.css` 和证据文件均低于
  1000 行；构建目录里的生成 xref 不属于运行源码门禁。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或
真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
