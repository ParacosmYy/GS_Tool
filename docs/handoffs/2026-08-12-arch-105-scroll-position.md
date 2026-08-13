# ARCH-105 / UI-1.178 交接：工作区滚动位置语义

日期：2026-08-12

## 结果

`WorkspaceScrollHint` 继续复用当前页面的 native vertical scrollbar，只增强 route strip 的
定位信息：

- 顶部：`顶部 · ↓ 向下查看`
- 中段：`中段 · ↕ 上下滚动`
- 底部：`底部 · ↑ 返回顶部`
- 无滚动：`全显 · 内容已全部显示`

提示的 accessible name 改为“工作区滚动位置”，动态 description 继续表达真实可用动作。
稳定 QSS 只将提示字重调整为 600，三主题的四态颜色 token、固定高度和 layout owner 不变。
没有新增 timer、thread、paint loop、scroll owner、业务状态、transport/session 或 OTA/debug
coupling。

## 证据

- `scripts/check.ps1`：pass；source limit `179 files <= 1000`，theme token audit pass，ruff pass。
- 真实 Qt offscreen：`star_trail`、`moonlit_ocean`、`sakura_night` × `980×720`、`1240×820` ×
  四个 workspace；所有可滚动页面的 0/中点/max 值分别得到 top/middle/bottom，横向 scroll
  `hmax=0`。
- 提示 geometry 稳定为 `(x=246, y=6, width=112, height=22)`；最长文案 font metrics 为
  `89px`，没有裁切或换行。
- focus 模式滚动到末尾仍得到 `bottom / 底部 · ↑ 返回顶部`；accessible name/description、
  context label 和 focus button 非空。
- shared MotionController 生命周期：normal/shown 为 `timer=True, can=True`；paused、disabled、
  hidden、closed 均为 `timer=False, can=False`；关闭后 `closed=True`，没有观察到重启。
- 最终视觉截图：
  `C:\Users\Gs\AppData\Local\Temp\serialforge-arch105-scroll-top.png`、
  `serialforge-arch105-scroll-middle.png`、`serialforge-arch105-scroll-bottom.png`。
- 本轮没有创建、修改或运行 unit test、mock、fixture、harness 或 test-only asset。

## 架构与审查记录

- 架构师 `019ff3d7-64c5-73e0-9fb4-bfa15c8d24cc` 在等待窗口内超时关闭，未形成外部结论，未伪造 PASS。
- 父代理复核：scrollbar 连接/解绑仍由 `WorkspaceScrollHint` 独占；业务 controller、滚动 owner、
  shared MotionController 和 theme token 边界未扩张；没有发现可安全合并的重复抽象。
- embedded C/C++ public-vendor-source applicability：N/A；本轮仅修改 Python/PySide6 presentation。

## 交付与未运行项

本轮使用 `local-arch-105` 完成 onefile 打包并覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,018,946` bytes，SHA-256 为
`02D61927294C1BEFC4A23F854125A478EE35CA0903A15A6BBD933B7D314665ED`，archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
EXE 启动、
真实 Windows 可见窗口、高刷新显示器/HIDPI、高负载、硬件连接、OTA/RTT 实连、签名和正式硬件
验收仍未运行或未授权；当前签名为 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。
