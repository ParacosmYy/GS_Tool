# UI Accessibility Evidence v143

日期：2026-08-12

## 变更范围

本轮处理最上方 `AI TOKEN` 品牌行在深色场景中看起来像黑色覆盖带的问题。运行时计算样式已经证明 header 的背景声明为透明；真正影响观感的是共享材质边界不够明确，以及旧 `5000` 启动链和完整 `5011` 启动链的 header 规则分散在多个样式模块中。

新增 `brand-clarity.css`，由两个入口共同加载的 `ui-polish.css` 引入。普通配色下 header、品牌子层和 Dashboard hero metadata rail 保持 `transparent / none / no blur`；只增加细微的上下光学边缘，背景插画仍然直接透过品牌行。forced-colors 仍恢复系统 Canvas/CanvasText，reduced-motion 关闭过渡。DOM、脚本、导航、认证、接口、Key 生命周期和数据流均未改变。

## 真实浏览器证据

| 入口 | 视口 | header 背景 | header blur | 文档宽度 | 页面日志 |
| --- | --- | --- | --- | --- | --- |
| `5000` | 320×720 | `rgba(0, 0, 0, 0)` | `none` | `305 / 305` | `[]` |
| `5000` | 390×844 | `rgba(0, 0, 0, 0)` | `none` | `375 / 375` | `[]` |
| `5000` | 768×900 | `rgba(0, 0, 0, 0)` | `none` | `753 / 753` | `[]` |
| `5000` | 1440×900 | `rgba(0, 0, 0, 0)` | `none` | `1425 / 1425` | `[]` |
| `5011` | 390×844 | `rgba(0, 0, 0, 0)` | `none` | `375 / 375` | `[]` |

两个入口的 `backgroundImage` 都为 `none`，没有重新引入黑色渐变或局部填充；`hero-topline` 同样保持透明，仅保留 `rgba(230, 235, 245, .11)` 的底部 hairline。移动端品牌行仍为 `76px` 高，场景背景、CTA 和滚动布局没有产生正向横向溢出。

## 可访问性与降级

- 仅新增视觉层规则，不新增焦点节点、tab 顺序、ARIA 属性或可交互区域。
- 透明材质不会以背景色承担文本语义；品牌文字继续使用既有颜色、keyline 和原生 focus ring。
- forced-colors 下 header、hero metadata rail 和子层恢复 `Canvas` / `CanvasText`，不依赖背景图片完成对比度。
- reduced-motion 下新增过渡被关闭；背景动效仍由既有场景模块独立处理。
- 修改后文件仍低于 1000 行：`brand-clarity.css` 83 行，`ui-polish.css` 985 行，`style.css` 674 行，`responsive-tuning.css` 693 行。

## 未覆盖发布门禁

本轮不涉及 Android、EXE 签名、真实 Provider、HTTPS/ACL、备份恢复、限流或真实设备辅助偏好；这些发布项继续按项目发布清单保持 pending。
