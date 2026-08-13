# ADR-0180：自适应工作区 Tab 文案契约

状态：Accepted  
日期：2026-08-12  
范围：workspace navigation presentation owner

## 背景

真实 Windows 字体度量下，窄窗口的四个工作区 Tab 会被原生 TabBar 挤压，出现滚动按钮和文案裁切。窗口缩窄是合法的响应式状态，不能依靠固定窗口最小宽度掩盖，也不能改变 route、业务页面或焦点语义。

## 决策

- `presentation/workspace_tab_surface.py::AnimatedWorkspaceTabBar` 是可见 Tab 文案投影的唯一 owner；它只接收完整/紧凑文案契约，不持有 ViewModel、route、transport、signals、timer 或业务状态。
- 按原生 `tabSizeHint()` 和所属 `QTabWidget.contentsRect()` 的实际可用宽度选择 `full -> compact -> icon`；`ElideNone` 保证不会再产生隐藏省略号。icon-only 是最后的保底模式，若仍不能满足原生布局则保留 Qt 原生 scroll buttons。
- 完整文案始终写入 `accessibleTabName` 与 tooltip；原生 tab index、icon、`currentChanged`、键盘焦点和导航顺序保持不变。
- `Resize`、`FontChange`、`StyleChange`、`LayoutRequest` 和父 TabWidget 的 show/resize 事件触发幂等重算。临时测量文案必须通过 `try/finally` 恢复，不得把候选文案遗留到可见状态。
- `workspace.py` 只负责安装自定义 TabBar、开启原生 scroll-button fallback 和注入四项文案契约；不把响应式策略扩散到页面 builder 或全局 stylesheet。

## 结果与证据

真实字体初始化为 `Microsoft YaHei UI` 的 Qt offscreen 组合根覆盖三主题、`1240/1120/980/768/640/600/560/546/520/480/420/360px`、四个页面，共 `144` 行：`>=600px` 为 full，`420–599px` 为 compact；每页逐一激活后 horizontal scrollbar maximum 均为 `0`，content 不超过 viewport，完整 accessibility name 与 tooltip 始终保留。`546px` 不再裁切，`546 -> 1240` 可恢复完整文案；icon fallback 保留但在本矩阵中 compact 已足够。

独立窗口事件循环采样 `240 frames / 113.954Hz`，证明共享 `MotionController` 的约 120Hz scheduler 在本环境仍可工作；这不是显示器/compositor 的精确 FPS 保证。静态门禁：Ruff、compileall、`scripts/check.ps1`、源码行数与主题 token audit 均通过。

架构师 `019ff646-9603-70e3-9d75-5e40bb286b44` 对 owner/API/范围及最终 `try/finally` 增量 `APPROVE`；独立代码审查 `019ff6a6-0113-7dd0-b7fd-699b006f7f09` `APPROVE`（Critical/Required=0）；简化评估 `019ff6a6-200d-7680-bd14-36b7fe5c829d` `APPROVE`（无必须简化项）。本轮未修改 embedded C/C++、固件或硬件，public-vendor-source applicability=N/A；未创建、修改或运行 unit test/mock/fixture/harness。

## 未覆盖边界

本轮使用非破坏性的离屏 Qt 组合根、静态检查和工程打包证据；没有运行持续真实 GUI/HIDPI/读屏、显示器 120Hz 合成、硬件链路、OTA/RTT/J-Link、代码签名或正式发行验收。
