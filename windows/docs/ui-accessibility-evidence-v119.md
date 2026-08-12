# UI 可访问性与视觉证据 · v119

## 变更主题

将 Dashboard `03 / AUTOMATIC COLLECTION` 旁的 `HOW IT WORKS` 说明收敛为桌面端 sticky protocol rail。自动采集表单约 884px 高，而说明内容约 492px；sticky rail 在表单中段保持三步协议可见，减少用户在输入过程中往返滚动。新增 `connect-rail.css` 只改变 presentation layout，不改变表单字段、API、Key 生命周期、DOM 语义或数据流。

隔离实例：`127.0.0.1:5220`，运行目录为 `windows/.cache/ui-v119-runtime-20260812-5220`。受保护服务 `5000/5011` 未触碰。

## 浏览器证据

- `1440×900` 深滚至自动采集区：协议栏计算样式为 `position: sticky; top: 96px`，在表单中段保持在阅读带内；左侧表单继续自然滚动。
- 协议栏视觉使用低 alpha 深蓝透景层、左侧 signal rail 和三步边界，不新增黑色横带；自动采集卡与协议栏的阅读层保持同一场景材质。
- 自动采集 Base URL 字段获得真实焦点后：表单保持 `opacity=1`，sticky 遮挡 mask 被 focus-within 解除，字段仍可完整操作。
- `320/390/768/1024/1440`：均无横向溢出；`901px` 以下协议栏为普通 `position: relative` 文档流，`390px` 长表单不被侧栏挤压。
- `390×844` 深滚：自动采集输入、检测模型、Prompt、备注和提交按钮维持触控布局；协议栏不会在表单视口中抢占空间。
- 应用页 console error/warn 数量为 `0`；浏览器宿主 telemetry warning 不计入应用错误。

## 动效与无障碍协议

- 信息：sticky rail 表达“协议随操作上下文保持可见”，左侧 signal rail 表达当前连接流程边界；不伪造连接状态。
- 触发：仅由原生滚动触发 CSS sticky，新增视觉层不增加监听器、计时器、网络请求或布局测量；布局成本为浏览器原生 sticky 合成。
- 降级：`901px` 以下关闭 sticky；`forced-colors` 使用 `Canvas/CanvasText`，移除 blur 和阴影；`prefers-reduced-motion` 关闭本模块过渡。
- 输入：保留既有 aside、标题、列表语义和 Tab 顺序；表单焦点通过既有 `:focus-within` 恢复 sticky occlusion 的完整阅读面。

## 角色复核

- UI：长表单与说明卡形成连续的“输入区 + 协议轨道”，减少滚动时的上下文丢失与两列材质断层。
- 前端：新增 `connect-rail.css`，只消费 `.tool-grid.auto-layout`、`.guide-card` 和既有 sticky occlusion contract；没有新增业务脚本或接口。
- 后端：无后端、数据库、接口、认证或密钥处理改动。
- 架构师：职责限定在 `04-connect`，桌面 sticky 与触屏普通流边界清晰；新增文件低于 1000 行，forced-colors/reduced-motion 和回滚路径明确。

## 未关闭门禁

真实设备、辅助偏好、Provider 联调、Android/EXE 构建、正式 HTTPS 与部署验收仍按项目总门禁执行，本证据不替代这些门禁。
