# ADR 0125：工作区当前页上下文表面

状态：Accepted（UI-1.137）  
日期：2026-08-11

## 背景

工作区路线条已经提供 Tab 路径、滚动可发现性和专注设置入口，但用户仍需要从 Tab
标题和页面内容中反复确认当前工作区。直接把页面业务状态、ViewModel 或导航策略塞进
路线条会扩大 shell 的耦合面，也会让未来增加嵌入式能力页时出现第二套状态源。

## 决策

新增 `presentation/workspace_context_surface.py:WorkspaceContextLabel`，它是一个只读、
鼠标透明、不可聚焦的 `QLabel`。它只把既有 `QTabWidget.currentChanged` 的 bounded index
投影为四个固定的 presentation context：

- `0`：链路配置
- `1`：解析与遥测
- `2`：命令管理
- `3`：能力预览

`controllers/workspace.py` 是唯一组装 owner；`WorkspaceShellBindings` 显式持有该 widget，
`workspace_runtime.py` 在既有 Tab change 回调中同步 index。label 不读取 ViewModel、session、
transport、OTA/debug DTO，不创建 timer、signal、导航动作或业务状态。无效 index 只安全收敛到
首个 context，启动早期缺失仍由 typed workspace bundle 的安全读取边界处理。

base/variant QSS 对 `workspaceContextLabel` 和四个 state selector 对称定义，颜色只来自
已有 ThemeSpec semantic token；布局使用 158～218px bounded width，route strip 保持固定
31px 高度。这样新增工作区页时，必须先扩展 bounded context catalog、Tab 与可访问文案，再
由 presentation owner 一起更新，而不是在多个 controller 中复制字符串判断。

## 验证与复核

真实 composition root 已覆盖三主题、980/1180、四 Tab 共 24 组：context 文案、state、
AccessibleDescription、bounded width、active page horizontal maximum=0 和 exact-white/
near-white=0 均通过；1180×780 连接页/扩展页截图已人工复核。compileall、Ruff、源码行数、
主题 token audit、项目既有检查和 onefile provenance 作为本轮交付门禁。

架构师角色调用：`019feddb-896c-7833-be05-c7d2052a767d`（Luna/max/Fast，限定等待窗口内
超时并关闭）；父代理完成 owner、依赖方向、index 边界、QSS 对称性、可访问性、生命周期和
简化审查。独立审查未返回，不计为独立通过。

嵌入式 C/C++ 适用性：N/A。本轮仅修改 Python/PySide6 presentation，不涉及固件、MCU、
BSP/HAL、RTOS、驱动、OTA 传输实现或硬件操作；不作 MISRA、ISO 或硬件合规声明。
